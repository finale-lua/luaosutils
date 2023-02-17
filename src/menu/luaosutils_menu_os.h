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
typedef HMENU menu_handle;
#else
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
typedef NSMenu * menu_handle;
#else
typedef void * menu_handle;
#endif
#endif

menu_handle __menu_findenclosing (const std::string& item_text, int starting_index, int& itemindex);
#endif /* luaosutils_menu_os_h */
