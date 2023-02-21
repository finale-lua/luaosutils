//
//  luaosutils_menu_os.h
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_menu_os_h
#define luaosutils_menu_os_h

#include <string>

#ifdef _MSC_VER
#include <windows.h>
typedef HWND window_handle;
typedef HMENU menu_handle;
#else
typedef void* window_handle;
typedef void* menu_handle;
#endif

enum class MENUITEM_TYPES
{
   ITEMTYPE_INVALID =      -1,
   ITEMTYPE_COMMAND =      0,
   ITEMTYPE_SUBMENU =      1,
   ITEMTYPE_SEPARATOR =    2
};

bool __menu_delete_submenu(menu_handle hMenu, window_handle hWnd);
menu_handle __menu_find_item(window_handle hWnd, const std::string& item_text, int starting_index, int& itemIndex);
int __menu_get_item_count(menu_handle hMenu);
menu_handle __menu_get_item_submenu(menu_handle hMenu, int index);
std::string __menu_get_item_text(menu_handle hMenu, int index);
MENUITEM_TYPES __menu_get_item_type(menu_handle hMenu, int index);
std::string __menu_get_title(menu_handle hMenu, window_handle hWnd);
menu_handle __menu_get_top_level_menu(window_handle hWnd);
int __menu_insert_separator(menu_handle hMenu, int insertIndex);
menu_handle __menu_insert_submenu(const std::string& itemText, menu_handle hMenu, int insertIndex, int& itemIndex);
bool __menu_move_item(menu_handle fromMenu, int fromIndex, menu_handle toMenu, int toIndex, int& itemIndex);
bool __menu_set_item_text(menu_handle hMenu, int index, const std::string& newText);
bool __menu_set_title(menu_handle hMenu, window_handle hWnd, const std::string& newText);

#endif /* luaosutils_menu_os_h */
