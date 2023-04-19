//
//  luaosutils_export.h
//  luaosutils
//
//  Created by Robert Patterson on 2/23/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_export_h
#define luaosutils_export_h

#include "lua.hpp"

#ifdef _WIN32 // can't use #if OPERATING_SYSTEM == WINDOWS here because external code may have conflicting definitions
#define LUAOSUTILS_EXPORT __declspec(dllexport)
#else
#define LUAOSUTILS_EXPORT __attribute__((visibility("default")))
#endif


#ifdef __cplusplus
extern "C" {
#endif

LUAOSUTILS_EXPORT int luaopen_luaosutils(lua_State* L);
LUAOSUTILS_EXPORT int luaopen_luaosutils_restricted(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif /* luaosutils_export_h */
