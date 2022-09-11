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
#include "luaosutils_os.h"
#include "luaosutils_callback_session.hpp"

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
static void __call_lua_function(luaosutils_callback_session &session, Args... args)
//static void __call_lua_function(luaosutils_callback_session& session, bool success, const std::string& urlResult)
{
   if (! luaosutils_callback_session::is_valid_session(&session)) // session has gone out of scope in Lua
      return;
   try
   {
      session.function()(args...);
   }
   catch (luabridge::LuaException &e)
   {
      LuaRun_AppendLineToOutput(e.state(), e.what());
      //ToDo: display any error message in a message box.
   }
}

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

   luaosutils_callback_session::id_type sessionID = luaosutils_callback_session::get_new_session_id();
   
   const OSSESSION_ptr os_session = __download_url(urlString,
          __download_callback([sessionID](bool success, const std::string &urlResult) -> void
         {
            luaosutils_callback_session* session = luaosutils_callback_session::get_session_for_id(sessionID);
            if (session)
            {
               __call_lua_function(*session, success, urlResult);
               session->set_os_session(nullptr);
            }
         }));

   if (os_session)
   {
      luaosutils_callback_session* session = new (lua_newuserdata(L, sizeof(luaosutils_callback_session)))
                                    luaosutils_callback_session(callback, sessionID);
      session->set_os_session(os_session);
      // Create a metatable for the userdata through that object can be accessed with "__gc". That means we get called when Lua state closes.
      lua_newtable(L);
      lua_pushstring(L, "__gc");
      lua_pushcfunction(L, [](lua_State* L)
      {
         auto udata = (luaosutils_callback_session*)lua_touserdata(L, 1);
         udata->~luaosutils_callback_session();
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
