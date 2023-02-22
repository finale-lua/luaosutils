//
//  libosutils_dylibmain.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/22/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
//  This file exists only to force XCode to link together the static libraries into a dylib.
//  It should not be included in the static library or when embedding in another program such as RGP Lua.

#include "luaosutils.hpp"

int dummy_func(lua_State *L);
int dummy_func(lua_State *L)
{
   return luaopen_luaosutils(L);
}
