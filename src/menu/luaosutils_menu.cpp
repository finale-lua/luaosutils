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

static int luaosutils_menu_find_item(lua_State *L)
{
   window_handle hWnd = reinterpret_cast<window_handle>(lua_touserdata(L, 1));
   std::string item_text = luabridge::Stack<std::string>::get(L, 2);
   luabridge::LuaRef min_index = luabridge::Stack<luabridge::LuaRef>::get(L, 3);

   int item_index = 0;
   menu_handle menu = __menu_find_item(hWnd, item_text, min_index.isNumber() ? min_index.cast<int>() : 0, item_index);
   if (! menu)
   {
      lua_pushnil(L);
      return 1;
   }
   
   lua_pushlightuserdata(L, menu);
   luabridge::Stack<int>::push(L, item_index);
   return 2;
}

static int luaosutils_menu_get_item_count(lua_State *L)
{
   menu_handle hMenu = reinterpret_cast<menu_handle>(lua_touserdata(L, 1));

   luabridge::Stack<int>::push(L, __menu_get_item_count(hMenu));
   return 1;
}

static int luaosutils_menu_get_item_text(lua_State *L)
{
   menu_handle hMenu = reinterpret_cast<menu_handle>(lua_touserdata(L, 1));
   int index = luabridge::Stack<int>::get(L, 2);
   
   luabridge::Stack<std::string>::push(L, __menu_get_item_text(hMenu, index));
   return 1;
}

static int luaosutils_menu_get_title(lua_State *L)
{
   menu_handle hMenu = reinterpret_cast<menu_handle>(lua_touserdata(L, 1));
   window_handle hWnd = reinterpret_cast<window_handle>(lua_touserdata(L, 2));

   luabridge::Stack<std::string>::push(L, __menu_get_title(hMenu, hWnd));
   return 1;
}

static int luaosutils_menu_get_top_level_menu(lua_State *L)
{
   window_handle hWnd = reinterpret_cast<window_handle>(lua_touserdata(L, 1));

   menu_handle menu = __menu_get_top_level_menu(hWnd);

   if (menu)
      lua_pushlightuserdata(L, menu);
   else
      lua_pushnil(L);

   return 1;
}

static int luaosutils_menu_set_item_text(lua_State *L)
{
   menu_handle hMenu = reinterpret_cast<menu_handle>(lua_touserdata(L, 1));
   int index = luabridge::Stack<int>::get(L, 2);
   std::string newText = luabridge::Stack<std::string>::get(L, 3);
   
   luabridge::Stack<bool>::push(L, __menu_set_item_text(hMenu, index, newText));
   return 1;
}

static int luaosutils_menu_set_title(lua_State *L)
{
   menu_handle hMenu = reinterpret_cast<menu_handle>(lua_touserdata(L, 1));
   window_handle hWnd = reinterpret_cast<window_handle>(lua_touserdata(L, 2));
   std::string newText = luabridge::Stack<std::string>::get(L, 3);

   luabridge::Stack<bool>::push(L, __menu_set_title(hMenu, hWnd, newText));
   return 1;
}

static const luaL_Reg menuutils[] = {
   {"find_item",           luaosutils_menu_find_item},
   {"get_item_count",      luaosutils_menu_get_item_count},
   {"get_item_text",       luaosutils_menu_get_item_text},
   {"get_title",           luaosutils_menu_get_title},
   {"get_top_level_menu",  luaosutils_menu_get_top_level_menu},
   {"set_item_text",       luaosutils_menu_set_item_text},
   {"set_title",           luaosutils_menu_set_title},
   {NULL, NULL} // sentinel
};

void luosutils_menu_create(lua_State *L)
{
   lua_newtable(L);  /* create metatable for file handles */
   luaL_setfuncs(L, menuutils, 0);  /* add file methods to new metatable */
   lua_setfield(L, -2, "menu");
}

