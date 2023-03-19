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
static void call_lua_function(luaosutils_callback_session &session, Args... args)
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
      error_message_box(e.what());
   }
}

static void create_luaosutils_callback_session(lua_State *L, OSSESSION_ptr os_session,
        luabridge::LuaRef callback, luaosutils_callback_session::id_type sessionID)
{
   luaosutils_callback_session* session = new (lua_newuserdata(L, sizeof(luaosutils_callback_session)))
                                 luaosutils_callback_session(callback, sessionID);
   session->set_os_session(os_session);
   // Create a metatable for the userdata through that object can be accessed with "gc". That means we get called when Lua state closes.
   lua_newtable(L);
   lua_pushstring(L, "gc");
   lua_pushcfunction(L, [](lua_State* L)
   {
      auto udata = (luaosutils_callback_session*)lua_touserdata(L, 1);
      udata->~luaosutils_callback_session();
      return 0;
   });
   lua_settable(L, -3);
   lua_setmetatable(L, -2);
}

/** \brief downloads the contents of a url into a string
 *
 * stack position 1: the url to download
 * stack position 2: a reference to a lua function to call on completion
 * stack position 3: optional HTTP headers
 * \return download session or nil
 */
static int luaosutils_internet_get(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string >(L, 1, LUA_TSTRING);
   auto callback = get_lua_parameter<luabridge::LuaRef>(L, 2, LUA_TFUNCTION);
   auto headers = get_lua_parameter<HeadersMap>(L, 3, LUA_TTABLE, HeadersMap());

   luaosutils_callback_session::id_type sessionID = luaosutils_callback_session::get_new_session_id();
   
   const OSSESSION_ptr os_session = https_request("get", urlString, "", headers, -1,
          lua_callback([sessionID](bool success, const std::string &urlResult) -> void
         {
            luaosutils_callback_session* session = luaosutils_callback_session::get_session_for_id(sessionID);
            if (session)
            {
               call_lua_function(*session, success, urlResult);
               session->set_os_session(nullptr);
            }
         }));

   if (os_session)
   {
      create_luaosutils_callback_session(L, os_session, callback, sessionID);
      return 1;
   }
   
   return 0;
}

/** \brief downloads the contents of a url into a string synchronously (blocks the UI)
 *
 * stack position 1: the url to download
 * stack position 2: a timeout value
 * stack position 3: optional HTTP headers
 * \return success
 * \return data or error message
 */
static int luaosutils_internet_get_sync(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string >(L, 1, LUA_TSTRING);
   auto timeout = (std::max)(0.0, get_lua_parameter<double>(L, 2, LUA_TNUMBER));
   auto headers = get_lua_parameter<HeadersMap>(L, 3, LUA_TTABLE, HeadersMap());

   bool success = false;
   std::string result;
   
   https_request("get", urlString, "", headers, timeout,
          lua_callback([&success, &result](bool cbsuccess, const std::string &data) -> void
         {
            success = cbsuccess;
            result = data;
         }));
   
   luabridge::Stack<bool>::push(L, success);
   luabridge::Stack<std::string>::push(L, result);
   return 2;
}


/** \brief post data to a url and returns the reply in a string
 *
 * stack position 1: the url to download
 * stack position 2: the post data (string)
 * stack position 3: a reference to a lua function to call on completion
 * stack position 4: optional HTTP headers
 * \return download session or nil
 */
static int luaosutils_internet_post(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string >(L, 1, LUA_TSTRING);
   auto postData = get_lua_parameter<std::string >(L, 2, LUA_TSTRING);
   auto callback = get_lua_parameter<luabridge::LuaRef>(L, 3, LUA_TFUNCTION);
   auto headers = get_lua_parameter<HeadersMap>(L, 4, LUA_TTABLE, HeadersMap());

   luaosutils_callback_session::id_type sessionID = luaosutils_callback_session::get_new_session_id();
   
   const OSSESSION_ptr os_session = https_request("post", urlString, postData, headers, -1,
          lua_callback([sessionID](bool success, const std::string &urlResult) -> void
         {
            luaosutils_callback_session* session = luaosutils_callback_session::get_session_for_id(sessionID);
            if (session)
            {
               call_lua_function(*session, success, urlResult);
               session->set_os_session(nullptr);
            }
         }));

   if (os_session)
   {
      create_luaosutils_callback_session(L, os_session, callback, sessionID);
      return 1;
   }
   
   return 0;
}


/** \brief downloads the contents of a url into a string synchronously (blocks the UI)
 *
 * stack position 1: the url to download
 * stack position 2: the post data (string)
 * stack position 3: a timeout value
 * stack position 4: optional HTTP headers
 * \return success
 * \return data or error message
 */

static int luaosutils_internet_post_sync(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string >(L, 1, LUA_TSTRING);
   auto postData = get_lua_parameter<std::string >(L, 2, LUA_TSTRING);
   auto timeout = (std::max)(0.0, get_lua_parameter<double>(L, 3, LUA_TNUMBER));
   auto headers = get_lua_parameter<HeadersMap>(L, 4, LUA_TTABLE, HeadersMap());

   bool success = false;
   std::string result;
   
   https_request("post", urlString, postData, headers, timeout,
          lua_callback([&success, &result](bool cbsuccess, const std::string &data) -> void
         {
            success = cbsuccess;
            result = data;
         }));
   
   luabridge::Stack<bool>::push(L, success);
   luabridge::Stack<std::string>::push(L, result);
   return 2;
}


static const luaL_Reg internet_utils[] = {
   {"download_url",        luaosutils_internet_get},        // alias for backwards compatibility
   {"download_url_sync",   luaosutils_internet_get_sync},   // alias for backwards compatibility
   {"get",                 luaosutils_internet_get},
   {"get_sync",            luaosutils_internet_get_sync},
   {"post",                luaosutils_internet_post},
   {"post_sync",           luaosutils_internet_post_sync},
   {NULL, NULL} // sentinel
};

void luaosutils_internet_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   luaL_setfuncs(L, internet_utils, 0);   // add file methods to new metatable
   lua_setfield(L, -2, "internet");       // add the nested table to the parent table with the name
}
