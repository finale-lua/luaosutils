//
//  luaosutils_menu.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//

#include "luaosutils.hpp"
#include "menu/luaosutils_menu.hpp"
#include "menu/luaosutils_menu_os.h"

static int luaosutils_menu_find_enclosing(lua_State *L)
{
   std::string item_text = luabridge::Stack<std::string>::get(L, 1);
   luabridge::LuaRef min_index = luabridge::Stack<luabridge::LuaRef>::get(L, 2);

   int item_index = 0;
   menu_handle the_menu = __menu_findenclosing (item_text, min_index.isNumber() ? min_index.cast<int>() : 0, item_index);
   if (! the_menu)
   {
      lua_pushnil(L);
      return 1;
   }
   
   lua_pushlightuserdata(L, the_menu);
   luabridge::Stack<int>::push(L, item_index);
   return 2;
}

static const luaL_Reg menuutils[] = {
   {"find_enclosing", luaosutils_menu_find_enclosing},
   {NULL, NULL} // sentinel
};


void luosutils_menu_create(lua_State *L)
{
   lua_newtable(L);  /* create metatable for file handles */
   luaL_setfuncs(L, menuutils, 0);  /* add file methods to new metatable */
   lua_setfield(L, -2, "menu");
}
