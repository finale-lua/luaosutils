//
//  luaosutils.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/9/2022.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include <string>
#include <exception>

#include "luaosutils.hpp"

#if OPERATING_SYSTEM == MAC_OS
#include "luaosutils_mac.h"
#endif

static void LuaRun_AppendLineToOutput(lua_State * L, const char * str)
{
   // I'm just guessing what this function should do, but this seems to make sense.
   if (! L)
      return;
   luabridge::LuaRef printFunc = luabridge::getGlobal(L, "print");
   if (printFunc.isFunction())
      printFunc(str);
}

template<typename... Args>
static void __call_lua_function(__luaosutils_callback_session &session, Args... args)
{
   if (! __luaosutils_callback_session::is_valid_session(&session)) // session has gone out of scope in Lua
      return;
   lua_gc(session.state(), LUA_GCSTOP, 0); // prevent garbage collection during callback
   try
   {
      session.function()(args...);
   }
   catch (luabridge::LuaException &e)
   {
      LuaRun_AppendLineToOutput(e.state(), e.what());
   }
   lua_gc(session.state(), LUA_GCRESTART, 0); // resume garbage collection after callback
   //ToDo: display any error message in a message box (after resuming GC)
}

__luaosutils_callback_session::~__luaosutils_callback_session()
{
   if (this->os_session())
   {
#if OPERATING_SYSTEM == MAC_OS
      __mac_cancel_http_request(this->os_session());
#endif
   }
   _get_active_sessions().erase(m_ID);
}

#if 0 //OPERATING_SYSTEM == WINDOWS
static DWORD RunWindowsThread(_In_ LPVOID lpParameter)
{
   lua_State * l = reinterpret_cast<lua_State *>(lpParameter);
   //do the download
   return 0;
}
#endif

/** \brief downloads the contents of a url into a string
 *
 * stack position 1: the url to download
 * stack position 2: a reference to a lua function to call on completion
 * \return download session or nil
 */
static int luaosutils_download_url (lua_State *L)
{
   std::string urlString = luabridge::Stack<std::string>::get(L, 1);
   luabridge::LuaRef callback = luabridge::Stack<luabridge::LuaRef>::get(L, 2);
   if (! callback.isFunction())
      luaL_error(L, "Function download_url expects a callback function in the second argument.");

   __luaosutils_callback_session::id_type sessionID = __luaosutils_callback_session::get_new_session_id();
   
#if OPERATING_SYSTEM == MAC_OS
   const OSSESSION_ptr os_session = __mac_download_url(urlString,
          __download_callback([sessionID](bool success, const std::string &urlResult) -> void
         {
            __luaosutils_callback_session* session = __luaosutils_callback_session::get_session_for_id(sessionID);
            if (session)
            {
               session->set_os_session(nullptr);
               __call_lua_function(*session, success, urlResult);
            }
         }));
#endif

   // On Windows, build the framework in a separate thread, because it needs a bigger stack than WinFin provides.
#if 0 //OPERATING_SYSTEM == WINDOWS
   HANDLE thread = CreateThread(nullptr, 0x400000, &RunWindowsThread, l, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);
   if (! thread)
      throw std::runtime_error("Unable to create thread to build Lua connection.");
   DWORD threadResult = WaitForSingleObject(thread, INFINITE);
   if (threadResult != WAIT_OBJECT_0)
      throw std::runtime_error("Thread ended with failure code.");
#endif

#if OPERATING_SYSTEM == WINDOWS
   OSSESSION_ptr os_session = nullptr;
#endif

   if (os_session)
   {
      __luaosutils_callback_session* session = new (lua_newuserdata(L, sizeof(__luaosutils_callback_session)))
                                    __luaosutils_callback_session(callback, sessionID);
      session->set_os_session(os_session);
      // Create a metatable for the userdata through that object can be accessed with "__gc". That means we get called when Lua state closes.
      lua_newtable(L);
      lua_pushstring(L, "__gc");
      lua_pushcfunction(L, [](lua_State* L)
      {
         auto udata = (__luaosutils_callback_session*)lua_touserdata(L, 1);
         udata->~__luaosutils_callback_session();
         return 0;
      });
      lua_settable(L, -3);
      lua_setmetatable(L, -2);
      return 1;
   }
   
   return 0;
}

static const luaL_Reg luaosutils[] = {
   {"download_url",     luaosutils_download_url},
   {NULL, NULL} // sentinel
};

int luaopen_luaosutils (lua_State *L) {
   /* export functions (and leave namespace table on top of stack) */
#if LUA_VERSION_NUM <= 501
   luaL_openlib(L, "luaosutils", luaosutils, 0);
#else
   luaL_newlib(L, luaosutils);
#endif
   /* make version string available to scripts */
   lua_pushstring(L, "_VERSION");
   lua_pushstring(L, LUAOSUTILS_VERSION);
   lua_rawset(L, -3);
   return 1;
}
