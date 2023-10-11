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
#include "internet/luaosutils_internet_lua.h"
#include "process/luaosutils_process_os.h"

template <>
struct LuaStack<luaosutils::HeadersMap> {
public:
   LuaStack(lua_State* L) : L(L) {}
   
   void push(const std::map<std::string, std::string>& value)
   {
      lua_newtable(L);
      for (const auto& entry : value) {
         LuaStack<std::string>(L).push(entry.first);
         LuaStack<std::string>(L).push(entry.second);
         lua_settable(L, -3);
      }
   }
   
   luaosutils::HeadersMap get(int index)
   {
      luaosutils::HeadersMap result;
      lua_pushnil(L);
      while (lua_next(L, index) != 0)
      {
         std::string key = LuaStack<std::string>(L).get(-2);
         std::string value = LuaStack<std::string>(L).get(-1);
         result[key] = value;
         lua_pop(L, 1);
      }
      return result;
   }
   
private:
   lua_State* L;
};

static void LuaRun_AppendLineToOutput(lua_State * L, const char * str)
{
   // I'm just guessing what this function should do, but this seems to make sense.
   if (! L)
      return;
   lua_getglobal(L, "print"); // get the global variable "print"
   if (lua_isfunction(L, -1)) // check if it is a function
   {
      lua_pushstring(L, str); // push the argument "str" onto the stack
      lua_call(L, 1, 0); // call the function with 1 argument and 0 return values
   }
   lua_pop(L, 1); // pop the function from the stack
}

template<typename... Args>
static void call_lua_function(luaosutils::callback_session &session, Args... args)
{
   if (! luaosutils::callback_session::is_valid_session(&session)) // session has gone out of scope in Lua
      return;
   lua_rawgeti(session.state(), LUA_REGISTRYINDEX, session.function());
   int nArgs = push_lua_args(session.state(), args...);
   int result = lua_pcall(session.state(), nArgs, 0, 0);
   if (result != LUA_OK)
   {
      const char* errorMessage = lua_tostring(session.state(), -1);
      LuaRun_AppendLineToOutput(session.state(), errorMessage);
#if defined(LUAOSUTILS_RGPLUA_AWARE)
      lua_getglobal(session.state(), "finenv");
      lua_getfield(session.state(), -1, "RetainLuaState");
      if (lua_isboolean(session.state(), -1))
      {
         lua_pushboolean(session.state(), 0);
         lua_setfield(session.state(), -3, "RetainLuaState");
      }
#endif // defined(LUAOSUTILS_RGPLUA_AWARE)
      luaosutils::error_message_box(errorMessage);
      lua_pop(session.state(), 1); // pop the error message from the stack
   }
}

static void create_luaosutils_callback_session(lua_State *L, luaosutils::OSSESSION_ptr& os_session,
           int callback, luaosutils::callback_session::id_type sessionID)
{
   luaosutils::callback_session* session = new (lua_newuserdata(L, sizeof(luaosutils::callback_session)))
                                 luaosutils::callback_session(L, callback, sessionID);
   session->set_os_session(os_session);
   // Create a metatable for the userdata through that object can be accessed with "gc". That means we get called when Lua state closes.
   lua_newtable(L);
   lua_pushstring(L, "__gc");
   lua_pushcfunction(L, [](lua_State* L)
   {
      auto udata = (luaosutils::callback_session*)lua_touserdata(L, 1);
      udata->~callback_session();
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
   auto urlString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto callback = get_lua_parameter<int>(L, 2, LUA_TFUNCTION);
   auto headers = get_lua_parameter<luaosutils::HeadersMap>(L, 3, LUA_TTABLE, luaosutils::HeadersMap());
   
   luaosutils::callback_session::id_type sessionID = luaosutils::callback_session::get_new_session_id();
      
   luaosutils::OSSESSION_ptr os_session = luaosutils::https_request("get", urlString, "", headers, -1,
         [sessionID, L, callback](bool success, const std::string &urlResult) -> void
         {
            luaosutils::callback_session* session = luaosutils::callback_session::get_session_for_id(sessionID);
            if (session)
            {
               call_lua_function(*session, success, urlResult);
               session->cancel();
            }
            else
            {
               luaosutils::callback_session temp(L, callback, luaosutils::callback_session::get_new_session_id());
               call_lua_function(temp, success, urlResult);
            }
         });

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
   auto urlString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto timeout = (std::max)(0.0, get_lua_parameter<double>(L, 2, LUA_TNUMBER));
   auto headers = get_lua_parameter<luaosutils::HeadersMap>(L, 3, LUA_TTABLE, luaosutils::HeadersMap());
   
   bool success = false;
   std::string result;
   
   luaosutils::https_request("get", urlString, "", headers, timeout,
            [&success, &result](bool cbsuccess, const std::string &data) -> void
                  {
                     success = cbsuccess;
                     result = data;
                  });
   
   LuaStack<bool>(L).push(success);
   LuaStack<std::string>(L).push(result);
   return 2;
}


/** \brief post data to a url and returns the reply in a string
 *
 * stack position 1: the url to post to
 * stack position 2: the post data (string)
 * stack position 3: a reference to a lua function to call on completion
 * stack position 4: optional HTTP headers
 * \return download session or nil
 */
int luaosutils_internet_post(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto postData = get_lua_parameter<std::string>(L, 2, LUA_TSTRING);
   auto callback = get_lua_parameter<int>(L, 3, LUA_TFUNCTION);
   auto headers = get_lua_parameter<luaosutils::HeadersMap>(L, 4, LUA_TTABLE, luaosutils::HeadersMap());
   
   luaosutils::callback_session::id_type sessionID = luaosutils::callback_session::get_new_session_id();
   
   luaosutils::OSSESSION_ptr os_session = luaosutils::https_request("post", urlString, postData, headers, -1,
         [sessionID, L, callback](bool success, const std::string &urlResult) -> void
         {
            luaosutils::callback_session* session = luaosutils::callback_session::get_session_for_id(sessionID);
            if (session)
            {
               call_lua_function(*session, success, urlResult);
               session->cancel();
            }
            else
            {
               luaosutils::callback_session temp(L, callback, luaosutils::callback_session::get_new_session_id());
               call_lua_function(temp, success, urlResult);
            }
      });

   if (os_session)
   {
      create_luaosutils_callback_session(L, os_session, callback, sessionID);
      return 1;
   }
   
   return 0;
}


/** \brief downloads the contents of a url into a string synchronously (blocks the UI)
 *
 * stack position 1: the url to post to
 * stack position 2: the post data (string)
 * stack position 3: a timeout value
 * stack position 4: optional HTTP headers
 * \return success
 * \return data or error message
 */

/** \brief launch the url in the user's default browsert
 *
 * stack position 1: the url to launch
 */
int luaosutils_internet_post_sync(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto postData = get_lua_parameter<std::string>(L, 2, LUA_TSTRING);
   auto timeout = (std::max)(0.0, get_lua_parameter<double>(L, 3, LUA_TNUMBER));
   auto headers = get_lua_parameter<luaosutils::HeadersMap>(L, 4, LUA_TTABLE, luaosutils::HeadersMap());
   
   bool success = false;
   std::string result;
   
   luaosutils::https_request("post", urlString, postData, headers, timeout,
                  [&success, &result](bool cbsuccess, const std::string &data) -> void
                              {
                                 success = cbsuccess;
                                 result = data;
                              });
   
   LuaStack<bool>(L).push(success);
   LuaStack<std::string>(L).push(result);
   return 2;
}

static int luaosutils_internet_cancel_session(lua_State* L)
{
   auto session = get_lua_parameter<luaosutils::callback_session*>(L, 1, LUA_TUSERDATA);
   if (session) session->cancel();
   return 0;
}

static int luaosutils_server_name(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   std::string result = luaosutils::server_name(urlString);
   LuaStack<std::string>(L).push(result);
   return 1;
}

static int luaosutils_launch_website(lua_State *L)
{
   auto urlString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);

   urlString = WINCODE("cmd /c start ") MACCODE("open ") + urlString;
   luaosutils::process_launch(urlString, "");
   
   return 0;
}

static const luaL_Reg internet_utils[] = {
   {"download_url",        luaosutils_internet_get},        // alias for backwards compatibility
   {"download_url_sync",   luaosutils_internet_get_sync},   // alias for backwards compatibility
   {"get",                 luaosutils_internet_get},
   {"get_sync",            luaosutils_internet_get_sync},
   {"post",                luaosutils_internet_post},
   {"post_sync",           luaosutils_internet_post_sync},
   {"cancel_session",      luaosutils_internet_cancel_session},
   {"launch_website",      luaosutils_launch_website},
   {"server_name",         luaosutils_server_name},
   {NULL, NULL} // sentinel
};

static const luaL_Reg internet_utils_restricted[] = {
   {"download_url",        restricted_function},         // alias for backwards compatibility
   {"download_url_sync",   restricted_function},         // alias for backwards compatibility
   {"get",                 restricted_function},
   {"get_sync",            restricted_function},
   {"post",                restricted_function},
   {"post_sync",           restricted_function},
   {"cancel_session",      restricted_function},
   {"launch_website",      luaosutils_launch_website},
   {"server_name",         luaosutils_server_name},
   {NULL, NULL} // sentinel
};

void luaosutils_internet_create(lua_State *L, bool restricted)
{
   lua_newtable(L);  // create nested table
   
   const luaL_Reg* funcs = restricted ? internet_utils_restricted : internet_utils;
   luaL_setfuncs(L, funcs, 0);            // add file methods to new metatable
   lua_setfield(L, -2, "internet");       // add the nested table to the parent table with the name
}
