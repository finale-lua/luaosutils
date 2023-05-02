//
//  luaosutils_process.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/21/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include "luaosutils.hpp"
#include "process/luaosutils_process_os.h"

static int luaosutils_process_execute(lua_State *L)
{
   auto cmd = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto dir = get_lua_parameter<std::string>(L, 2, LUA_TSTRING, std::string());
   
   if (cmd.size())
   {
      std::string output;
      const bool result = luaosutils::process_execute(cmd, dir, output);
      if (result)
         push_lua_return_value(L, output);
      else
         lua_pushnil(L);
   }
   else
      lua_pushnil(L);

   return 1;
}

static int luaosutils_process_launch(lua_State *L)
{
   auto cmd = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto dir = get_lua_parameter<std::string>(L, 2, LUA_TSTRING, std::string());

   if (cmd.size())
      push_lua_return_value(L, luaosutils::process_launch(cmd, dir));
   else
      push_lua_return_value(L, false);
   return 1;
}

static int luaosutils_process_make_dir(lua_State *L)
{
   auto pathString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto dir = get_lua_parameter<std::string>(L, 2, LUA_TSTRING, std::string());

   const std::string mkdirString = std::string(WINCODE("cmd /c mkdir ") MACCODE("mkdir ")) + '"' + pathString + '"';
   push_lua_return_value(L, luaosutils::process_launch(mkdirString, dir));
   
   return 1;
}

static int luaosutils_process_list_dir(lua_State *L)
{
   auto pathString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto options = get_lua_parameter<std::string>(L, 2, LUA_TSTRING, std::string());
   if (options.size())
      options = ' ' + options;

   const std::string lsdirString = std::string(WINCODE("cmd /c dir") MACCODE("ls")) + options;
   std::string output;
   const bool result = luaosutils::process_execute(lsdirString, pathString, output);
   if (result)
      push_lua_return_value(L, output);
   else
      lua_pushnil(L);

   return 1;
}


static const luaL_Reg process_utils[] = {
   {"execute",             luaosutils_process_execute},
   {"launch",              luaosutils_process_launch},
   {"make_dir",            luaosutils_process_make_dir},
   {"list_dir",            luaosutils_process_list_dir},
   {NULL, NULL} // sentinel
};

static const luaL_Reg process_utils_restricted[] = {
   {"execute",             restricted_function},
   {"launch",              restricted_function},
   {"make_dir",            luaosutils_process_make_dir},
   {"list_dir",            luaosutils_process_list_dir},
   {NULL, NULL} // sentinel
};

void luaosutils_process_create(lua_State *L, bool restricted)
{
   lua_newtable(L);  // create nested table
   
   const luaL_Reg* funcs = restricted ? process_utils_restricted : process_utils;
   luaL_setfuncs(L, funcs, 0);         // add file methods to new metatable
   lua_setfield(L, -2, "process");     // add the nested table to the parent table with the name
}
