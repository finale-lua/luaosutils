//
//  luaosutils_menu.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include "luaosutils.hpp"
#include "menu/luaosutils_menu_os.h"

static int luaosutils_menu_delete_submenu(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   window_handle hWnd = __get_lua_parameter<window_handle>(L, 2, nullptr);
   
   if (__menu_get_item_count(hMenu) > 0)
      __push_lua_return_value(L, false);
   else
      __push_lua_return_value(L, __menu_delete_submenu(hMenu, hWnd));
   return 1;
}

static int luaosutils_menu_find_item(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   std::string itemText = __get_lua_parameter(L, 2, std::string());
   
   if (itemText.size() <= 0)
   {
      lua_pushnil(L);
      return 1;
   }

   int itemIndex = 0;
   menu_handle menu = __menu_find_item(hMenu, itemText, 0, itemIndex);
   if (! menu)
   {
      lua_pushnil(L);
      return 1;
   }
   
   __push_lua_return_value(L, menu);
   __push_lua_return_value(L, itemIndex);
   return 2;
}

static int luaosutils_menu_get_item_command_id(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int index = __get_lua_parameter(L, 2, -1);

   if (index < 0 || !hMenu || index >= __menu_get_item_count(hMenu))
      lua_pushnil(L);
   else
   {
      long retval = __menu_get_item_command_id(hMenu, index);
      if (retval > 0)
         __push_lua_return_value(L, retval);
      else
         lua_pushnil(L);
   }
   return 1;
}

static int luaosutils_menu_get_item_count(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);

   __push_lua_return_value(L, __menu_get_item_count(hMenu));
   return 1;
}

static int luaosutils_menu_get_item_submenu(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int index = __get_lua_parameter(L, 2, -1);

   if (index < 0 || !hMenu || index >= __menu_get_item_count(hMenu))
      lua_pushnil(L);
   else
      __push_lua_return_value(L, __menu_get_item_submenu(hMenu, index));
   return 1;
}

static int luaosutils_menu_get_item_text(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int index = __get_lua_parameter(L, 2, -1);
   
   __push_lua_return_value(L, __menu_get_item_text(hMenu, index));
   return 1;
}

static int luaosutils_get_item_type(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int index = __get_lua_parameter(L, 2, -1);

   if (index < 0 || !hMenu || index >= __menu_get_item_count(hMenu))
      __push_lua_return_value(L, static_cast<int>(MENUITEM_TYPES::ITEMTYPE_INVALID));
   else
      __push_lua_return_value(L, static_cast<int>(__menu_get_item_type(hMenu, index)));
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

static int luaosutils_menu_insert_separator(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int insertIndex = __get_lua_parameter(L, 2, -1);
   
   if (!hMenu)
   {
      lua_pushnil(L);
      return 1;
   }

   int itemIndex = __menu_insert_separator(hMenu, insertIndex);

   if (itemIndex < 0)
      lua_pushnil(L);
   else
      __push_lua_return_value(L, itemIndex);
   return 1;
}

static int luaosutils_menu_insert_submenu(lua_State *L)
{
   std::string itemText = __get_lua_parameter(L, 1, std::string());
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 2, nullptr);
   int insertIndex = __get_lua_parameter(L, 3, -1);
   
   if (itemText.size() <= 0 || !hMenu)
   {
      lua_pushnil(L);
      return 1;
   }

   int itemIndex = 0;
   menu_handle submenu = __menu_insert_submenu(itemText, hMenu, insertIndex, itemIndex);
   if (! submenu)
   {
      lua_pushnil(L);
      return 1;
   }
   
   __push_lua_return_value(L, submenu);
   __push_lua_return_value(L, itemIndex);
   return 2;
}

static int luaosutils_menu_move_item(lua_State *L)
{
   menu_handle fromMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int fromIndex = __get_lua_parameter(L, 2, -1);
   menu_handle toMenu = __get_lua_parameter<menu_handle>(L, 3, nullptr);
   int toIndex = __get_lua_parameter(L, 4, -1);
   
   if (fromIndex < 0 || !fromMenu || fromIndex >= __menu_get_item_count(fromMenu) || !toMenu)
   {
      __push_lua_return_value(L, false);
      return 1;
   }
   
   int itemIndex = 0;
   bool result = __menu_move_item(fromMenu, fromIndex, toMenu, toIndex, itemIndex);
   
   __push_lua_return_value(L, result);
   if (! result)
      return 1;
   __push_lua_return_value(L, itemIndex);
   return 2;
}

static int luaosutils_menu_redraw([[maybe_unused]]lua_State* L)
{
#ifdef _MSC_VER
   window_handle hWnd = __get_lua_parameter<window_handle>(L, 1, nullptr);
   if (hWnd) DrawMenuBar(hWnd);
#endif

   return 0;
}

static int luaosutils_menu_set_item_text(lua_State *L)
{
   menu_handle hMenu = __get_lua_parameter<menu_handle>(L, 1, nullptr);
   int index = __get_lua_parameter(L, 2, -1);
   std::string newText = __get_lua_parameter(L, 3, std::string());
   
   if (newText.size() <= 0 || index < 0 || index >= __menu_get_item_count(hMenu))
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

static const std::map<std::string, MENUITEM_TYPES> __constants =
{
   {"ITEMTYPE_INVALID",       MENUITEM_TYPES::ITEMTYPE_INVALID},
   {"ITEMTYPE_COMMAND",       MENUITEM_TYPES::ITEMTYPE_COMMAND},
   {"ITEMTYPE_SUBMENU",       MENUITEM_TYPES::ITEMTYPE_SUBMENU},
   {"ITEMTYPE_SEPARATOR",     MENUITEM_TYPES::ITEMTYPE_SEPARATOR}
};

static const luaL_Reg menu_utils[] = {
   {"delete_submenu",      luaosutils_menu_delete_submenu},
   {"find_item",           luaosutils_menu_find_item},
   {"get_item_command_id", luaosutils_menu_get_item_command_id},
   {"get_item_count",      luaosutils_menu_get_item_count},
   {"get_item_submenu",    luaosutils_menu_get_item_submenu},
   {"get_item_text",       luaosutils_menu_get_item_text},
   {"get_item_type",       luaosutils_get_item_type},
   {"get_title",           luaosutils_menu_get_title},
   {"get_top_level_menu",  luaosutils_menu_get_top_level_menu},
   {"insert_separator",    luaosutils_menu_insert_separator},
   {"insert_submenu",      luaosutils_menu_insert_submenu},
   {"move_item",           luaosutils_menu_move_item},
   {"redraw",              luaosutils_menu_redraw},
   {"set_item_text",       luaosutils_menu_set_item_text},
   {"set_title",           luaosutils_menu_set_title},
   {NULL, NULL} // sentinel
};

void luaosutils_menu_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   for (auto constant : __constants)
      __add_constant(L, constant.first.c_str(), static_cast<int>(constant.second), -3);
   
   luaL_setfuncs(L, menu_utils, 0);  // add file methods to new metatable
   lua_setfield(L, -2, "menu");     // add the nested table to the parent table with the name
}

