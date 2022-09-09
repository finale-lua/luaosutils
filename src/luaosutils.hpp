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

#if defined(__GNUC__)
#define OPERATING_SYSTEM MAC_OS
#elif defined(_MSC_VER)
#define OPERATING_SYSTEM WINDOWS
#else
#define OPERATING_SYSTEM UNKNOWN
#endif

#include <stdio.h>
#include <string>
#include <functional>

#include "lua.hpp"

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

using __download_callback = std::function<void (bool, const std::string&)>;

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT int luaopen_luaosutils(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif // luaosutils_hpp
