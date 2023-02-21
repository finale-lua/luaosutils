//
//  luaosutils_menu_os_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 2/17/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#import <Cocoa/Cocoa.h>

#include "menu/luaosutils_menu_os.h"

NSMenuItem* __GetEnclosingMenuItemForMenu(const NSMenu * const subMenu)
{
   NSMenu * superMenu = [subMenu supermenu];
   if ( nil != superMenu )
   {
      const NSInteger count = [superMenu numberOfItems];
      for ( int i = 0; i < count; i++ )
      {
         NSMenuItem * item = [superMenu itemAtIndex:i];
         if ( item && [item hasSubmenu] && (subMenu == [item submenu]) )
            return item;
      }
   }
   return nil;
}

bool __menu_delete_submenu(menu_handle hMenu, window_handle)
{
   NSMenu* subMenu = (__bridge NSMenu*)hMenu;
   @try
   {
      NSMenuItem* menuItem = __GetEnclosingMenuItemForMenu(subMenu);
      if (menuItem)
      {
         [[subMenu supermenu] removeItem:menuItem];
         return true;
      }
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_delete_submenu: %@", exception);
   }
   return false;
}

menu_handle __menu_find_item (window_handle hWnd, const std::string& item_text, int starting_index, int& itemIndex)
{
   // Search for the first menu item that starts with the input text
   NSInteger startingIndex = starting_index;
   NSString *itemText = [NSString stringWithUTF8String:item_text.c_str()];
   
   auto searchSubmenus = [&itemIndex, itemText](menu_handle menu, NSInteger startIndex, auto&& searchSubmenus) -> menu_handle
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
            itemIndex = (int)i;
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
      NSLog(@"Caught exception in __menu_get_item_count: %@", exception);
   }
   return 0;
}

menu_handle __menu_get_item_submenu(menu_handle hMenu, int index)
{
   NSMenu* menu = (__bridge NSMenu*)hMenu;
   @try
   {
      NSMenuItem* item = [menu itemAtIndex:index];
      if (item && [item hasSubmenu])
         return (__bridge menu_handle)[item submenu];
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_get_item_type: %@", exception);
   }
   return nullptr;

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
      NSLog(@"Caught exception in __menu_get_item_text: %@", exception);
   }
   return "";
}

MENUITEM_TYPES __menu_get_item_type(menu_handle hMenu, int index)
{
   NSMenu* menu = (__bridge NSMenu*)hMenu;
   @try
   {
      NSMenuItem* item = [menu itemAtIndex:index];
      if (item)
      {
         if ([item hasSubmenu]) return MENUITEM_TYPES::ITEMTYPE_SUBMENU;
         if ([item isSeparatorItem]) return MENUITEM_TYPES::ITEMTYPE_SEPARATOR;
         return MENUITEM_TYPES::ITEMTYPE_COMMAND;
      }
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_get_item_type: %@", exception);
   }
   return MENUITEM_TYPES::ITEMTYPE_INVALID;
}

std::string __menu_get_title(menu_handle hMenu, window_handle)
{
   NSMenu* menu = (__bridge NSMenu*)hMenu;
   @try
   {
      NSMenuItem* item = __GetEnclosingMenuItemForMenu(menu);
      if (item)
         return [[item title] UTF8String];
      return [[menu title] UTF8String];
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_get_text: %@", exception);
   }
   return "";
}

menu_handle __menu_get_top_level_menu(window_handle)
{
   return (__bridge menu_handle)[[NSApplication sharedApplication] mainMenu];
}

int __menu_insert_separator(menu_handle hMenu, int insertIndex)
{
   NSMenu* menu =  (__bridge NSMenu*)hMenu;
   @try
   {
      NSInteger itemIndex = -1;
      if (insertIndex >= 0)
      {
         itemIndex = std::min<NSInteger>(insertIndex, [menu numberOfItems]);
         [menu insertItem:[NSMenuItem separatorItem] atIndex:itemIndex];
      }
      else
      {
         itemIndex = static_cast<int>([menu numberOfItems]);
         [menu addItem:[NSMenuItem separatorItem]];
      }
      return static_cast<int>(itemIndex);

   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_insert_separator: %@", exception);
   }
   return -1;
}

menu_handle __menu_insert_submenu(const std::string& itemText, menu_handle hMenu, int insertIndex, int& itemIndex)
{
   NSMenu* menu =  (__bridge NSMenu*)hMenu;
   @try
   {
      NSString* title = [NSString stringWithUTF8String:itemText.c_str()];
      NSMenuItem * subMenuItem = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
      [subMenuItem setEnabled:true];
      [subMenuItem setTitle:title];
      NSMenu * subMenu = [[NSMenu alloc] initWithTitle:title];
      [subMenuItem setSubmenu:subMenu];
      if (insertIndex >= 0)
      {
         itemIndex = static_cast<int>(std::min<NSInteger>(insertIndex, [menu numberOfItems]));
         [menu insertItem:subMenuItem atIndex:itemIndex];
      }
      else
      {
         itemIndex = static_cast<int>([menu numberOfItems]);
         [menu addItem:subMenuItem];
      }
      return (__bridge menu_handle)subMenu;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_insert_submenu: %@", exception);
   }
   return nil;
}

bool __menu_move_item(menu_handle fromMenu, int fromIndex, menu_handle toMenu, int toIndex, int& itemIndex)
{
   NSMenu* nsFromMenu = (__bridge NSMenu*)fromMenu;
   NSMenu* nsToMenu = (__bridge NSMenu*)toMenu;
   @try
   {
      NSMenuItem* fromItem = [nsFromMenu itemAtIndex:fromIndex];
      if ([fromItem submenu]) return false;
      NSMenuItem* newItem = [fromItem copy];
      [newItem setMenu:nil];
      if (toIndex < 0)
      {
         itemIndex = static_cast<int>([nsToMenu numberOfItems]);
         [nsToMenu addItem:newItem];
      }
      else
      {
         itemIndex = static_cast<int>(std::min<NSInteger>(toIndex, [nsToMenu numberOfItems]));
         [nsToMenu insertItem:newItem atIndex:itemIndex];
      }
      [nsFromMenu removeItem:fromItem];
      return true;
   } @catch (NSException *exception)
   {
      NSLog (@"Exception In MenuUtilities AppendMenuItem: %@", exception);
   }
   return false;
}

bool __menu_set_item_text(menu_handle hMenu, int index, const std::string& newText)
{
   NSMenu* menu = (__bridge NSMenu*)hMenu;
   @try
   {
      NSMenuItem* menuItem = [menu itemAtIndex:index];
      if (! menuItem) return false;
      if ([menuItem isSeparatorItem]) return false;
      [menuItem setTitle:[NSString stringWithUTF8String:newText.c_str()]];
      return true;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_get_item_text%@", exception);
   }
   return false;
}

bool __menu_set_title(menu_handle hMenu, window_handle hWnd, const std::string& newText)
{
   NSMenu* menu = (__bridge NSMenu*)hMenu;
   @try
   {
      [menu setTitle:[NSString stringWithUTF8String:newText.c_str()]];
      NSMenuItem *menuItem = __GetEnclosingMenuItemForMenu(menu);
      if (menuItem)
         [menuItem setTitle:[NSString stringWithUTF8String:newText.c_str()]];
      return true;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __menu_get_text: %@", exception);
   }
   return false;
}
