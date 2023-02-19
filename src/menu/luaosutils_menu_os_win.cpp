//
//  luaosutils_menu_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright � 2023 Robert Patterson. All rights reserved.
//
#include <vector>

#include <windows.h>

#include "luaosutils.hpp"
#include "menu/luaosutils_menu_os.h"

static std::basic_string<WCHAR> __utf8_to_WCHAR(const char * inpstr)
{
	std::basic_string<WCHAR> retval;

	const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, inpstr, -1, nullptr, 0) - 1; // remove null-terminator
	if (size > 0)
	{
		retval.resize(size);
		MultiByteToWideChar(CP_UTF8, 0, inpstr, -1, retval.data(), size);
	}
	return retval;
}

static std::string __WCHAR_to_utf8(const WCHAR* inpstr)
{
	std::string retval;

	int size = WideCharToMultiByte(CP_UTF8, 0, inpstr, -1, NULL, 0, NULL, NULL) - 1; // remove null-terminator
	if (size > 0)
	{
		retval.resize(size);
		WideCharToMultiByte(CP_UTF8, 0, inpstr, -1, retval.data(), size, NULL, NULL);
	}
	return retval;
}

static HMENU __GetParentMenu(HMENU menu, HMENU origin, int* pos)
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
			parent = __GetParentMenu(menu, parent, pos);
			if (parent) return parent;
		}
	}
	return NULL;
}

menu_handle __menu_find_item(window_handle hWnd, const std::string& item_text, int starting_index, int& itemindex)
{
   // Search for the first menu item that starts with the input text
	std::basic_string<WCHAR> itemText = __utf8_to_WCHAR(item_text.c_str());
   HMENU topMenu = __menu_get_top_level_menu(hWnd);

   auto searchSubmenus = [&itemindex, itemText](HMENU hMenu, int startIndex, auto&& searchSubmenus) -> HMENU
   {
		const int maxItems = __menu_get_item_count(hMenu);
		
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
				itemindex = i;
				return hMenu;
			}
		}
      return nullptr;
   };

   return searchSubmenus(topMenu, starting_index, searchSubmenus);
}

int __menu_get_item_count(menu_handle hMenu)
{
	return hMenu ? GetMenuItemCount(hMenu) : 0;
}

std::string __menu_get_item_text(menu_handle hMenu, int index)
{
	WCHAR menuText[1024];
	GetMenuStringW(hMenu, index, menuText, DIM(menuText) - 1, MF_BYPOSITION);
	menuText[DIM(menuText) - 1] = 0;
	return __WCHAR_to_utf8(menuText);
}

std::string __menu_get_title(menu_handle hMenu, window_handle hWnd)
{
	int parentIndex = 0;
	HMENU hParentMenu = __GetParentMenu(hMenu, __menu_get_top_level_menu(hWnd), &parentIndex);
	if (hParentMenu)
		return __menu_get_item_text(hParentMenu, parentIndex);

	return "";
}

menu_handle __menu_get_top_level_menu(window_handle hWnd)
{
	return GetMenu(hWnd);
}

bool __menu_move_item(menu_handle fromMenu, int fromIndex, menu_handle toMenu, int toIndex)
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
			InsertMenuW(toMenu, toIndex, MF_BYPOSITION | MF_STRING, menuInfo.wID, menuText);
		else
			AppendMenuW(toMenu, MF_STRING, menuInfo.wID, menuText);
		return true;
	}
	return false;
}

bool __menu_set_item_text(menu_handle hMenu, int index, const std::string& newText)
{
	std::basic_string<WCHAR> newTextW = __utf8_to_WCHAR(newText.c_str());
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

bool __menu_set_title(menu_handle hMenu, window_handle hWnd, const std::string& newText)
{
	int parentIndex = 0;
	HMENU hParentMenu = __GetParentMenu(hMenu, __menu_get_top_level_menu(hWnd), &parentIndex);
	if (hParentMenu)
		return __menu_set_item_text(hParentMenu, parentIndex, newText);
	return false;
}
