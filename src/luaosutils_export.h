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

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT int luaopen_luaosutils(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif /* luaosutils_export_h */
