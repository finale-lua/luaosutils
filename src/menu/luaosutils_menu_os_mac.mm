//
//  luaosutils_menu_os_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//
#import <Cocoa/Cocoa.h>

#include "menu/luaosutils_menu_os.h"

menu_handle __menu_findenclosing (const std::string& item_text, int starting_index, int& itemindex)
{
   // Search for the first menu item that starts with the input text
   NSInteger startingIndex = starting_index;
   NSString *itemText = [NSString stringWithUTF8String:item_text.c_str()];
   
   auto searchSubmenus = [&itemindex, itemText](NSMenu* menu, NSInteger startIndex, auto&& searchSubmenus) -> NSMenu*
   {
      for (NSInteger i = startIndex; i < [menu numberOfItems]; i++)
      {
         NSMenuItem* item = [menu itemAtIndex:i];
         if (item.submenu)
         {
            NSMenu* subResult = searchSubmenus(item.submenu, 0, searchSubmenus);
            if (subResult) return subResult;
         }
         if ([item.title hasPrefix:itemText])
         {
            itemindex = (int)i;
            return [item menu];
         }
      }
      return nil;
   };
   
   return searchSubmenus([[NSApplication sharedApplication] mainMenu], startingIndex, searchSubmenus);
}
