//
//  luaosutils_internet_lua.h
//  luaosutils-static
//
//  Created by Robert Patterson on 4/26/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
//  This header allows RGP Lua to directly call the lua versions of the post functions,
//  allowing for intermediate security around post calls.

#ifndef luaosutils_internet_lua_h
#define luaosutils_internet_lua_h

#include "lua.hpp"

int luaosutils_internet_post(lua_State *L);
int luaosutils_internet_post_sync(lua_State *L);

#endif /* luaosutils_internet_lua_h */
