//
//  luaosutils_internet.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/21/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include <string>
#include <exception>

#include "luaosutils.hpp"
#include "internet/luaosutils_callback_session.hpp"

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
#if defined(LUAOSUTILS_RGPLUA_AWARE)
      luabridge::LuaRef finenv = luabridge::getGlobal(e.state(), "finenv");
      if (finenv["RetainLuaState"].isBool())
         finenv["RetainLuaState"] = false;
#endif // defined(LUAOSUTILS_RGPLUA_AWARE)
      __error_message_box(e.what());
   }
}

/** \brief downloads the contents of a url into a string
 *
 * stack position 1: the url to download
 * stack position 2: a reference to a lua function to call on completion
 * \return download session or nil
 */
static int luaosutils_download_url(lua_State *L)
{
   std::string urlString = luabridge::Stack<std::string>::get(L, 1);
   luabridge::LuaRef callback = luabridge::Stack<luabridge::LuaRef>::get(L, 2);
   if (! callback.isFunction())
      luaL_error(L, "Function download_url expects a callback function in the second argument.");

   luaosutils_callback_session::id_type sessionID = luaosutils_callback_session::get_new_session_id();
   
   const OSSESSION_ptr os_session = __download_url(urlString, -1,
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

/** \brief downloads the contents of a url into a string synchronously (blocks the UI)
 *
 * stack position 1: the url to download
 * stack position 2: a timeout value
 * \return success
 * \return data or error message
 */
static int luaosutils_download_url_sync(lua_State *L)
{
   std::string urlString = luabridge::Stack<std::string>::get(L, 1);
   double timeout = (std::max)(0.0, luabridge::Stack<double>::get(L, 2));
   
   bool success = false;
   std::string result;
   
   __download_url(urlString, timeout,
          __download_callback([&success, &result](bool cbsuccess, const std::string &data) -> void
         {
            success = cbsuccess;
            result = data;
         }));
   
   luabridge::Stack<bool>::push(L, success);
   luabridge::Stack<std::string>::push(L, result);
   return 2;
}

static const luaL_Reg internet_utils[] = {
   {"download_url",        luaosutils_download_url},
   {"download_url_sync",   luaosutils_download_url_sync},
   {NULL, NULL} // sentinel
};

void luaosutils_internet_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   luaL_setfuncs(L, internet_utils, 0);   // add file methods to new metatable
   lua_setfield(L, -2, "internet");       // add the nested table to the parent table with the name
}
