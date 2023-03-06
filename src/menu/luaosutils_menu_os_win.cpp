//
//  luaosutils_menu_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#include <vector>

#include <windows.h>

#include "luaosutils.hpp"
#include "menu/luaosutils_menu_os.h"
#include "winutils/luaosutils_winutils.h"

static HMENU getParentMenu(HMENU menu, HMENU origin, int* pos)
{
	if (!origin) return NULL;
	if (origin == menu) return NULL;
	for (int i = 0, n = GetMenuItemCount(origin); i < n; i++)
	{
		HMENU parent = GetSubMenu(origin, i);
		if (parent == menu)
		{
			if (pos) *pos = i;
			return origin;
		}
		else if (parent)
		{
			parent = getParentMenu(menu, parent, pos);
			if (parent) return parent;
		}
	}
	return NULL;
}

bool menu_delete_submenu(menu_handle hMenu, window_handle hWnd)
{
	if (0 == GetMenuItemCount(hMenu))
	{
		int superIndex = 0;
		HMENU superMenu = getParentMenu(hMenu, menu_get_top_level_menu(hWnd), &superIndex);
		if (superMenu)
		{
			DeleteMenu(superMenu, superIndex, MF_BYPOSITION);
			return true;
		}
	}
	return false;
}

menu_handle menu_find_item(menu_handle hMenu, const std::string& item_text, int starting_index, int& itemIndex)
{
   // Search for the first menu item that starts with the input text
	std::basic_string<WCHAR> itemText = utf8_to_WCHAR(item_text.c_str());

   auto searchSubmenus = [&itemIndex, itemText](HMENU hMenu, int startIndex, auto&& searchSubmenus) -> HMENU
   {
		const int maxItems = menu_get_item_count(hMenu);
		
		for (int i = startIndex; i < maxItems; i++)
      {
			HMENU hSubMenu = GetSubMenu(hMenu, i);
			if (nullptr != hSubMenu)
			{
				HMENU retVal = searchSubmenus(hSubMenu, 0, searchSubmenus);
				if (retVal) return retVal;
			}
			WCHAR menuText[1024];
			GetMenuStringW(hMenu, i, menuText, DIM(menuText) - 1, MF_BYPOSITION);
			menuText[DIM(menuText) - 1] = 0;
			// Can't use strcmp because we need to ignore "&" char
			bool gotIt = true;
			for (int x1 = 0, x2 = 0; itemText[x1]; x1++, x2++)
			{
				if ('&' == menuText[x2])
					x2++;
				if (itemText[x1] != menuText[x2])
				{
					gotIt = false;
					break;
				}
			}
			if (gotIt)
			{
				itemIndex = i;
				return hMenu;
			}
		}
      return nullptr;
   };

   return searchSubmenus(hMenu, starting_index, searchSubmenus);
}

long menu_get_item_command_id(menu_handle hMenu, int index)
{
	return hMenu ? GetMenuItemID(hMenu, index) : -1;
}

int menu_get_item_count(menu_handle hMenu)
{
	return hMenu ? GetMenuItemCount(hMenu) : 0;
}

menu_handle menu_get_item_submenu(menu_handle hMenu, int index)
{
	return GetSubMenu(hMenu, index);
}

std::string menu_get_item_text(menu_handle hMenu, int index)
{
	WCHAR menuText[1024];
	if (!GetMenuStringW(hMenu, index, menuText, DIM(menuText) - 1, MF_BYPOSITION))
		return "";
	menuText[DIM(menuText) - 1] = 0;
	return WCHAR_to_utf8(menuText);
}

MENUITEM_TYPES menu_get_item_type(menu_handle hMenu, int index)
{
	if (GetSubMenu(hMenu, index))
		return MENUITEM_TYPES::ITEMTYPE_SUBMENU;
	if (GetMenuState(hMenu, index, MF_BYPOSITION) & MF_SEPARATOR)
		return MENUITEM_TYPES::ITEMTYPE_SEPARATOR;
	return MENUITEM_TYPES::ITEMTYPE_COMMAND;
}

std::string menu_get_title(menu_handle hMenu, window_handle hWnd)
{
	int parentIndex = 0;
	HMENU hParentMenu = getParentMenu(hMenu, menu_get_top_level_menu(hWnd), &parentIndex);
	if (hParentMenu)
		return menu_get_item_text(hParentMenu, parentIndex);
	return "";
}

menu_handle menu_get_top_level_menu(window_handle hWnd)
{
	return GetMenu(hWnd);
}

int menu_insert_separator(menu_handle hMenu, int insertIndex)
{
	int retval = -1;
	if (insertIndex < 0)
	{
		retval = GetMenuItemCount(hMenu);
		if (!AppendMenuW(hMenu, MF_SEPARATOR, NULL, NULL))
			return -1;
	}
	else
	{
		retval = (std::min)(insertIndex, GetMenuItemCount(hMenu));
		if (!InsertMenuW(hMenu, retval, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL))
			return -1;
	}
	return retval;
}	

menu_handle menu_insert_submenu(const std::string& itemText, menu_handle hMenu, int insertIndex, int& itemIndex)
{
	HMENU newSubMenu = CreatePopupMenu();
	if (!newSubMenu)
		return NULL;
	std::basic_string<WCHAR> itemTextW = utf8_to_WCHAR(itemText.c_str());
	if (insertIndex < 0)
	{
		itemIndex = GetMenuItemCount(hMenu);
		if (!AppendMenuW(hMenu, MF_STRING | MF_POPUP | MF_ENABLED, (UINT_PTR)newSubMenu, itemTextW.c_str()))
			return nullptr;
	}
	else
	{
		itemIndex = (std::min)(insertIndex, GetMenuItemCount(hMenu));
		if (!InsertMenuW(hMenu, itemIndex, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_ENABLED, (UINT_PTR)newSubMenu, itemTextW.c_str()))
			return nullptr;
	}
	return newSubMenu;
}

bool menu_move_item(menu_handle fromMenu, int fromIndex, menu_handle toMenu, int toIndex, int& itemIndex)
{
	MENUITEMINFOW menuInfo;
	memset(&menuInfo, 0, sizeof(menuInfo));
	menuInfo.cbSize = sizeof(menuInfo);
	menuInfo.fMask = MIIM_ID | MIIM_STRING;
	WCHAR menuText[1024];
	menuInfo.dwTypeData = menuText;
	menuInfo.cch = DIM(menuText);
	if (GetMenuItemInfoW(fromMenu, fromIndex, true, &menuInfo) && (NULL == menuInfo.hSubMenu))
	{
		DeleteMenu(fromMenu, fromIndex, MF_BYPOSITION);
		if (toIndex >= 0)
		{
			itemIndex = (std::min)(toIndex, GetMenuItemCount(toMenu));
			InsertMenuW(toMenu, itemIndex, MF_BYPOSITION | MF_STRING, menuInfo.wID, menuText);
		}
		else
		{
			itemIndex = GetMenuItemCount(toMenu);
			AppendMenuW(toMenu, MF_STRING, menuInfo.wID, menuText);
		}
		return true;
	}
	return false;
}

bool menu_set_item_text(menu_handle hMenu, int index, const std::string& newText)
{
	std::basic_string<WCHAR> newTextW = utf8_to_WCHAR(newText.c_str());
	const int commandID = static_cast<int>(GetMenuItemID(hMenu, index));
	if (commandID == -1)
	{
		MENUITEMINFOW menuItemInfo;
		memset(&menuItemInfo, 0, sizeof(menuItemInfo));
		menuItemInfo.cbSize = sizeof(menuItemInfo);
		menuItemInfo.fMask = MIIM_STRING;
		menuItemInfo.dwTypeData = (LPWSTR)newTextW.c_str();
		return SetMenuItemInfoW(hMenu, index, MF_BYPOSITION, &menuItemInfo);
	}
	else
	{
		//for some reason, Finale does not recognize these changes (for non submenu items)
		//if we use SetMenuItemInfoW. It may have to do with timing of the calls, but anyway this works.
		DeleteMenu(hMenu, index, MF_BYPOSITION);
		InsertMenuW(hMenu, index, MF_BYPOSITION | MF_STRING, commandID, (LPWSTR)newTextW.c_str());
		return true;
	}
}

bool menu_set_title(menu_handle hMenu, window_handle hWnd, const std::string& newText)
{
	int parentIndex = 0;
	HMENU hParentMenu = getParentMenu(hMenu, menu_get_top_level_menu(hWnd), &parentIndex);
	if (hParentMenu)
		return menu_set_item_text(hParentMenu, parentIndex, newText);
	return false;
}
