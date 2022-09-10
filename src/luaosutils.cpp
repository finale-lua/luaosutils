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

   __luaosutils_callback_session* session = new __luaosutils_callback_session(callback);

#if OPERATING_SYSTEM == MAC_OS
   const bool success = __mac_download_url(urlString,
          __download_callback([session](bool success, const std::string &urlResult) -> void
         {
            __call_lua_function(*session, success, urlResult);
         }));
#endif

   if (success)
   {
      auto udata = (__luaosutils_callback_session*)lua_newuserdata(L, sizeof(__luaosutils_callback_session));
      memcpy(udata, session, sizeof(__luaosutils_callback_session));
      // Create a metatable for the userdata through that object can be access in 2 ways- "__gc" and "__index"
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
   
   delete session;
   return 0;
}

static const luaL_Reg luaosutils[] = {
   {"download_url",     luaosutils_download_url},
   {NULL, NULL} // sentinel
};

int luaopen_luaosutils (lua_State *L) {
#if LUA_VERSION_NUM <= 501
   luaL_openlib(L, "luaosutils", luaosutils, 0);
#else
   luaL_newlib(L, luaosutils);
#endif
   return 1;
}
