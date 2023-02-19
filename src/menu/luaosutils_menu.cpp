//
//  luaosutils_menu.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//

#include "luaosutils.hpp"
#include "menu/luaosutils_menu_os.h"

static int luaosutils_menu_find_item(lua_State *L)
{
   window_handle hWnd = __get_lua_parameter<window_handle>(L, 1, nullptr);
   std::string itemText = __get_lua_parameter(L, 2, std::string());
   int minIndex = __get_lua_parameter(L, 3, 0);
   
   if (itemText.size() <= 0)
   {
      lua_pushnil(L);
      return 1;
   }

   int itemIndex = 0;
   menu_handle menu = __menu_find_item(hWnd, itemText, minIndex, itemIndex);
   if (! menu)
   {
      lua_pushnil(L);
      return 1;
   }
   
   __push_lua_return_value(L, menu);
   __push_lua_return_value(L, itemIndex);
   return 2;
}

static int luaosutils_menu_get_item_count(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);

   __push_lua_return_value(L, __menu_get_item_count(hMenu));
   return 1;
}

static int luaosutils_menu_get_item_text(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int index = __get_lua_parameter(L, 2, -1);
   
   __push_lua_return_value(L, __menu_get_item_text(hMenu, index));
   return 1;
}

static int luaosutils_menu_get_title(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   window_handle hWnd = __get_lua_parameter<window_handle>(L, 2, nullptr);

   __push_lua_return_value(L, __menu_get_title(hMenu, hWnd));
   return 1;
}

static int luaosutils_menu_get_top_level_menu(lua_State *L)
{
   window_handle hWnd = __get_lua_parameter<window_handle>(L, 1, nullptr);

   __push_lua_return_value(L, __menu_get_top_level_menu(hWnd));
   return 1;
}

static int luaosutils_menu_move_item(lua_State *L)
{
   menu_handle fromMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int fromIndex = __get_lua_parameter(L, 2, -1);
   menu_handle toMenu = __get_lua_parameter<menu_handle>(L, 3, nullptr);
   int toIndex = __get_lua_parameter(L, 4, -1);
   
   if (fromIndex < 0 || !fromMenu || !toMenu)
      __push_lua_return_value(L, false);
   else
      __push_lua_return_value(L, __menu_move_item(fromMenu, fromIndex, toMenu, toIndex));
   return 1;
}


static int luaosutils_menu_set_item_text(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int index = __get_lua_parameter(L, 2, -1);
   std::string newText = __get_lua_parameter(L, 3, std::string());
   
   if (newText.size() <= 0 || index < 0)
      __push_lua_return_value(L, false);
   else
      __push_lua_return_value(L, __menu_set_item_text(hMenu, index, newText));
   return 1;
}

static int luaosutils_menu_set_title(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   window_handle hWnd = __get_lua_parameter<window_handle>(L, 2, nullptr);
   std::string newText = __get_lua_parameter(L, 3, std::string());

   if (newText.size() <= 0)
      __push_lua_return_value(L, false);
   else
      __push_lua_return_value(L, __menu_set_title(hMenu, hWnd, newText));
   return 1;
}

static const luaL_Reg menuutils[] = {
   {"find_item",           luaosutils_menu_find_item},
   {"get_item_count",      luaosutils_menu_get_item_count},
   {"get_item_text",       luaosutils_menu_get_item_text},
   {"get_title",           luaosutils_menu_get_title},
   {"get_top_level_menu",  luaosutils_menu_get_top_level_menu},
   {"move_item",           luaosutils_menu_move_item},
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

