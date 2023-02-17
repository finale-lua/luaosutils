//
//  luaosutils_menu_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright � 2023 Robert Patterson. All rights reserved.
//
#include <vector>

#include <windows.h>

#include "menu/luaosutils_menu_os.h"

#define DIM(a) (sizeof(a)/sizeof(a[0]))

#define WM_MDIGETCLIENT (WM_USER+8)

static std::basic_string<WCHAR> __utf8_to_WCHAR(const char * inpstr)
{
	std::basic_string<WCHAR> retval;

	const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, inpstr, -1, nullptr, 0);
	if (size > 0)
	{
		retval.resize(size);
		MultiByteToWideChar(CP_UTF8, 0, inpstr, -1, retval.data(), size);
	}
	return retval;
}

BOOL CALLBACK __enumChildWindowsProc(HWND hWnd, LPARAM lParam)
{
   // Get the MDI client window of the current top-level window, if it has one
   HWND hMDIClient = (HWND)SendMessage(hWnd, WM_MDIGETCLIENT, 0, 0);
   if (hMDIClient != NULL)
   {
      // Save the MDI client window handle in the vector pointed to by lParam
      std::vector<HWND>* pVec = reinterpret_cast<std::vector<HWND>*>(lParam);
      pVec->push_back(hMDIClient);
   }

   return TRUE;
}

static HWND __getMDIClientWindow()
{
   // Enumerate all top-level windows in the current process
   std::vector<HWND> mdiClients;
   EnumWindows(__enumChildWindowsProc, reinterpret_cast<LPARAM>(&mdiClients));

   // Print the handle of the first MDI client window found (if any) to the console
   if (!mdiClients.empty())
      return mdiClients[0];

   return nullptr;
}

static HMENU __getMainMenu()
{
	HWND hWnd = __getMDIClientWindow();
   HMENU hMenu = GetMenu(hWnd);
   if (hMenu) return hMenu;

	hWnd = GetForegroundWindow();
	hMenu = GetMenu(hWnd);
   return hMenu;
}

menu_handle __menu_findenclosing(const std::string& item_text, int starting_index, int& itemindex)
{
   // Search for the first menu item that starts with the input text
	std::basic_string<WCHAR> itemText = __utf8_to_WCHAR(item_text.c_str());
   HMENU topMenu = __getMainMenu();

   auto searchSubmenus = [&itemindex, itemText](HMENU hMenu, int startIndex, auto&& searchSubmenus) -> HMENU
   {
		const int maxItems = GetMenuItemCount(hMenu);
		
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