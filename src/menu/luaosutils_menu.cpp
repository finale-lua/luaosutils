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
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA, nullptr);
   auto hWnd = get_lua_parameter<window_handle>(L, 2, LUA_TLIGHTUSERDATA MAC_PARM(nullptr));
   
   if (menu_get_item_count(hMenu) > 0)
      push_lua_return_value(L, false);
   else
      push_lua_return_value(L, menu_delete_submenu(hMenu, hWnd));
   return 1;
}

static int luaosutils_menu_find_item(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto itemText = get_lua_parameter<std::string>(L, 2, LUA_TSTRING);
   auto startIndex = get_lua_parameter<int>(L, 3, LUA_TNUMBER, 0);
   
   if (itemText.size() <= 0)
   {
      lua_pushnil(L);
      return 1;
   }

   int itemIndex = 0;
   menu_handle menu = menu_find_item(hMenu, itemText, startIndex, itemIndex);
   if (! menu)
   {
      lua_pushnil(L);
      return 1;
   }
   
   push_lua_return_value(L, menu);
   push_lua_return_value(L, itemIndex);
   return 2;
}

static int luaosutils_menu_get_item_command_id(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto index = get_lua_parameter<int>(L, 2, LUA_TNUMBER);

   if (index < 0 || !hMenu || index >= menu_get_item_count(hMenu))
      lua_pushnil(L);
   else
   {
      long retval = menu_get_item_command_id(hMenu, index);
      if (retval > 0)
         push_lua_return_value(L, retval);
      else
         lua_pushnil(L);
   }
   return 1;
}

static int luaosutils_menu_get_item_count(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);

   push_lua_return_value(L, menu_get_item_count(hMenu));
   return 1;
}

static int luaosutils_menu_get_item_submenu(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto index = get_lua_parameter<int>(L, 2, LUA_TNUMBER);

   if (index < 0 || !hMenu || index >= menu_get_item_count(hMenu))
      lua_pushnil(L);
   else
      push_lua_return_value(L, menu_get_item_submenu(hMenu, index));
   return 1;
}

static int luaosutils_menu_get_item_text(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto index = get_lua_parameter<int>(L, 2, LUA_TNUMBER);
   
   push_lua_return_value(L, menu_get_item_text(hMenu, index));
   return 1;
}

static int luaosutils_get_item_type(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto index = get_lua_parameter<int>(L, 2, LUA_TNUMBER);

   if (index < 0 || !hMenu || index >= menu_get_item_count(hMenu))
      push_lua_return_value(L, static_cast<int>(MENUITEM_TYPES::ITEMTYPE_INVALID));
   else
      push_lua_return_value(L, static_cast<int>(menu_get_item_type(hMenu, index)));
   return 1;
}

static int luaosutils_menu_get_title(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto hWnd = get_lua_parameter<window_handle>(L, 2, LUA_TLIGHTUSERDATA MAC_PARM(nullptr));

   push_lua_return_value(L, menu_get_title(hMenu, hWnd));
   return 1;
}

static int luaosutils_menu_get_top_level_menu(lua_State *L)
{
   auto hWnd = get_lua_parameter<window_handle>(L, 1, LUA_TLIGHTUSERDATA MAC_PARM(nullptr));

   push_lua_return_value(L, menu_get_top_level_menu(hWnd));
   return 1;
}

static int luaosutils_menu_insert_separator(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto insertIndex = get_lua_parameter<int>(L, 2, LUA_TNUMBER, -1);
   
   if (!hMenu)
   {
      lua_pushnil(L);
      return 1;
   }

   int itemIndex = menu_insert_separator(hMenu, insertIndex);

   if (itemIndex < 0)
      lua_pushnil(L);
   else
      push_lua_return_value(L, itemIndex);
   return 1;
}

static int luaosutils_menu_insert_submenu(lua_State *L)
{
   auto itemText = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto hMenu = get_lua_parameter<menu_handle>(L, 2, LUA_TLIGHTUSERDATA);
   auto insertIndex = get_lua_parameter<int>(L, 3, LUA_TNUMBER, -1);
   
   if (itemText.size() <= 0 || !hMenu)
   {
      lua_pushnil(L);
      return 1;
   }

   int itemIndex = 0;
   menu_handle submenu = menu_insert_submenu(itemText, hMenu, insertIndex, itemIndex);
   if (! submenu)
   {
      lua_pushnil(L);
      return 1;
   }
   
   push_lua_return_value(L, submenu);
   push_lua_return_value(L, itemIndex);
   return 2;
}

static int luaosutils_menu_move_item(lua_State *L)
{
   auto fromMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto fromIndex = get_lua_parameter<int>(L, 2, LUA_TNUMBER);
   auto toMenu = get_lua_parameter<menu_handle>(L, 3, LUA_TLIGHTUSERDATA);
   auto toIndex = get_lua_parameter<int>(L, 4, LUA_TNUMBER, -1);
   
   if (fromIndex < 0 || !fromMenu || fromIndex >= menu_get_item_count(fromMenu) || !toMenu)
   {
      push_lua_return_value(L, false);
      return 1;
   }
   
   int itemIndex = 0;
   bool result = menu_move_item(fromMenu, fromIndex, toMenu, toIndex, itemIndex);
   
   push_lua_return_value(L, result);
   if (! result)
      return 1;
   push_lua_return_value(L, itemIndex);
   return 2;
}

static int luaosutils_menu_redraw([[maybe_unused]]lua_State* L)
{
#if OPERATING_SYSTEM == WINDOWS
   window_handle hWnd = get_lua_parameter<window_handle>(L, 1, LUA_TLIGHTUSERDATA, nullptr);
   if (hWnd) DrawMenuBar(hWnd);
#endif

   return 0;
}

static int luaosutils_menu_set_item_text(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto index = get_lua_parameter<int>(L, 2, LUA_TNUMBER);
   auto newText = get_lua_parameter<std::string>(L, 3, LUA_TSTRING);
   
   if (newText.size() <= 0 || index < 0 || index >= menu_get_item_count(hMenu))
      push_lua_return_value(L, false);
   else
      push_lua_return_value(L, menu_set_item_text(hMenu, index, newText));
   return 1;
}

static int luaosutils_menu_set_title(lua_State *L)
{
   auto hMenu = get_lua_parameter<menu_handle>(L, 1, LUA_TLIGHTUSERDATA);
   auto hWnd = get_lua_parameter<window_handle>(L, 2, LUA_TLIGHTUSERDATA MAC_PARM(nullptr));
   auto newText = get_lua_parameter<std::string>(L, 3, LUA_TSTRING);

   if (newText.size() <= 0)
      push_lua_return_value(L, false);
   else
      push_lua_return_value(L, menu_set_title(hMenu, hWnd, newText));
   return 1;
}

static const std::map<std::string, MENUITEM_TYPES> constants =
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
   
   for (auto constant : constants)
      add_constant(L, constant.first.c_str(), static_cast<int>(constant.second), -3);
   
   luaL_setfuncs(L, menu_utils, 0);  // add file methods to new metatable
   lua_setfield(L, -2, "menu");     // add the nested table to the parent table with the name
}

