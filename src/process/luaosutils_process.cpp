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
   auto cmd = __get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto dir = __get_lua_parameter<std::string>(L, 2, LUA_TSTRING, std::string());
   
   if (cmd.size())
   {
      std::string output;
      const bool result = __process_execute(cmd, dir, output);
      if (result)
         __push_lua_return_value(L, output);
      else
         lua_pushnil(L);
   }
   else
      lua_pushnil(L);

   return 1;
}

static int luaosutils_process_launch(lua_State *L)
{
   auto cmd = __get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto dir = __get_lua_parameter<std::string>(L, 2, LUA_TSTRING, std::string());

   if (cmd.size())
      __push_lua_return_value(L, __process_launch(cmd, dir));
   else
      __push_lua_return_value(L, false);
   return 1;
}

static const luaL_Reg process_utils[] = {
   {"execute",             luaosutils_process_execute},
   {"launch",              luaosutils_process_launch},
   {NULL, NULL} // sentinel
};

void luaosutils_process_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   luaL_setfuncs(L, process_utils, 0);   // add file methods to new metatable
   lua_setfield(L, -2, "process");       // add the nested table to the parent table with the name
}
