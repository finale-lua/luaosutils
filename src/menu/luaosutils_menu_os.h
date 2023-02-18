//
//  luaosutils_menu_os.h
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
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

menu_handle __menu_find_item(window_handle hWnd, const std::string& item_text, int starting_index, int& itemindex);
int __menu_get_item_count(menu_handle hMenu);
std::string __menu_get_item_text(menu_handle hMenu, int index);
std::string __menu_get_title(menu_handle hMenu, window_handle hWnd);
menu_handle __menu_get_top_level_menu(window_handle hWnd);

#endif /* luaosutils_menu_os_h */
