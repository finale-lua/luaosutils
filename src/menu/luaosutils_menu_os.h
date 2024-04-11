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

#include "luaosutils.hpp"


namespace luaosutils
{

#if OPERATING_SYSTEM == WINDOWS
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

bool menu_delete_submenu(menu_handle hMenu, window_handle hWnd);
bool menu_execute_command_id(long cmd, window_handle hWnd);
menu_handle menu_find_item(menu_handle hMenu, const std::string& item_text, int starting_index, int& itemIndex);
long menu_get_item_command_id(menu_handle hMenu, int index);
int menu_get_item_count(menu_handle hMenu);
menu_handle menu_get_item_submenu(menu_handle hMenu, int index);
std::string menu_get_item_text(menu_handle hMenu, int index);
MENUITEM_TYPES menu_get_item_type(menu_handle hMenu, int index);
std::string menu_get_title(menu_handle hMenu, window_handle hWnd);
menu_handle menu_get_top_level_menu(window_handle hWnd);
int menu_insert_separator(menu_handle hMenu, int insertIndex);
menu_handle menu_insert_submenu(const std::string& itemText, menu_handle hMenu, int insertIndex, int& itemIndex);
bool menu_move_item(menu_handle fromMenu, int fromIndex, menu_handle toMenu, int toIndex, int& itemIndex);
bool menu_set_item_text(menu_handle hMenu, int index, const std::string& newText);
bool menu_set_title(menu_handle hMenu, window_handle hWnd, const std::string& newText);
inline void menu_redraw(window_handle hWnd)
{
#if OPERATING_SYSTEM == WINDOWS
   if (hWnd) DrawMenuBar(hWnd);
#endif
}

}

#endif /* luaosutils_menu_os_h */
