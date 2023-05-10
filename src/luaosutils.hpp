//
//  luaosutils.hpp
//  luaosutils
//
//  Created by Robert Patterson on 9/9/2022.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_hpp
#define luaosutils_hpp

#define LUAOSUTILS_VERSION "Luaosutils 2.2.1"

#define MAC_OS       1         /* Macintosh operating system */
#define WINDOWS      2         /* Microsoft Windows (MS-DOS) */
#define UNKNOWN_OS   -1

#if defined(_WIN32)
#define OPERATING_SYSTEM WINDOWS
#elif defined(__GNUC__)
#define OPERATING_SYSTEM MAC_OS
#else
#define OPERATING_SYSTEM UNKNOWN_OS
#endif

#if OPERATING_SYSTEM == WINDOWS
#define WINCODE(X) X
#define WIN_PARM(X) , X
#else
#define WINCODE(X)
#define WIN_PARM(X)
#endif

#if OPERATING_SYSTEM == MAC_OS
#define MACCODE(X) X
#define MAC_PARM(X) , X
#else
#define MACCODE(X)
#define MAC_PARM(X)
#endif

#include <string>
#include <functional>
#include <optional>

#include "lua.hpp"
#include "luaosutils_export.h"
#include "luaosutils_luastack.h"

#define TRUSTED_ERROR_MESSAGE "trusted code is required to run this function"

//utility functions

#define DIM(a) (sizeof(a)/sizeof(a[0]))

inline void add_constant(lua_State *L, const char* const_name, int value, int table_index)
{
   lua_pushstring(L, const_name);
   lua_pushinteger(L, value);
   lua_rawset(L, table_index);
}

inline int restricted_function(lua_State *L)
{
   luaL_error(L, "the current permissions environment does not allow this function to run");
   return 0;
}

void luaosutils_internet_create(lua_State *L, bool restricted);
void luaosutils_menu_create(lua_State *L, bool restricted);
void luaosutils_process_create(lua_State *L, bool restricted);
void luaosutils_text_create(lua_State *L);

#endif // luaosutils_hpp
