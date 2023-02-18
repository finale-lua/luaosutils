//
//  luaosutils_menu_os_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//
#import <Cocoa/Cocoa.h>

#include "menu/luaosutils_menu_os.h"

menu_handle __menu_find_item (window_handle hWnd, const std::string& item_text, int starting_index, int& itemindex)
{
   // Search for the first menu item that starts with the input text
   NSInteger startingIndex = starting_index;
   NSString *itemText = [NSString stringWithUTF8String:item_text.c_str()];
   
   auto searchSubmenus = [&itemindex, itemText](menu_handle menu, NSInteger startIndex, auto&& searchSubmenus) -> menu_handle
   {
      const int itemCount = __menu_get_item_count(menu);
      for (NSInteger i = startIndex; i < itemCount; i++)
      {
         NSMenuItem* item = [(__bridge NSMenu *)menu itemAtIndex:i];
         if (item.submenu)
         {
            menu_handle subResult = searchSubmenus((__bridge menu_handle)item.submenu, 0, searchSubmenus);
            if (subResult) return subResult;
         }
         if ([item.title hasPrefix:itemText])
         {
            itemindex = (int)i;
            return (__bridge menu_handle)[item menu];
         }
      }
      return nil;
   };
   
   return searchSubmenus(__menu_get_top_level_menu(hWnd), startingIndex, searchSubmenus);
}

int __menu_get_item_count(menu_handle hMenu)
{
   NSMenu* menu = (__bridge NSMenu*)hMenu;
   @try
   {
      return (int)[menu numberOfItems];
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_get_item_count%@", exception);
   }
   return 0;
}

std::string __menu_get_item_text(menu_handle hMenu, int index)
{
   NSMenu* menu = (__bridge NSMenu*)hMenu;
   @try
   {
      NSMenuItem* menuItem = [menu itemAtIndex:index];
      if (! menuItem) return "";
      if ([menuItem isSeparatorItem]) return "-";
      return [[menuItem title] UTF8String];
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_get_item_text%@", exception);
   }
   return "";
}

menu_handle __menu_get_top_level_menu(window_handle)
{
   return (__bridge menu_handle)[[NSApplication sharedApplication] mainMenu];
}
