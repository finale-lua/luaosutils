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

#define LUAOSUTILS_VERSION "Luaosutils 1.1.1"

#define MAC_OS       1         /* Macintosh operating system */
#define WINDOWS      2         /* Microsoft Windows (MS-DOS) */

#if defined(__GNUC__)
#define OPERATING_SYSTEM MAC_OS
#elif defined(_MSC_VER)
#define OPERATING_SYSTEM WINDOWS
#else
#define OPERATING_SYSTEM UNKNOWN
#endif

#include <string>
#include <functional>

#include "lua.hpp"

#ifndef __OBJC__
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif // __GNUC__
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedObject.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // __GNUC__
#endif // __OBJC__

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

//ToDo: __download_callback should be in url/luaosutils_url.cpp
using __download_callback = std::function<void (bool, const std::string&)>;

//utility functions

#define DIM(a) (sizeof(a)/sizeof(a[0]))

#ifndef __OBJC__
template<typename T>
T __get_lua_parameter(lua_State* L, int paramNum, T defaultValue)
{
   if constexpr (std::is_convertible<T, void*>::value)
   {
      T ptr = reinterpret_cast<T>(lua_touserdata(L, paramNum));
      if (!ptr) return defaultValue;
      return ptr;
   }
   else
   {
      luabridge::LuaRef ref = luabridge::Stack<luabridge::LuaRef>::get(L, paramNum);
      if (ref.isNil()) return defaultValue;
      return ref.cast<T>();
   }
}

template<typename T>
void __push_lua_return_value(lua_State* L, T retval)
{
   if constexpr (std::is_convertible<T, void*>::value)
   {
      if (! retval)
         lua_pushnil(L);
      else
         lua_pushlightuserdata(L, retval);
   }
   else
      luabridge::Stack<T>::push(L, retval);
}
#endif

void luosutils_menu_create(lua_State *L);

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT int luaopen_luaosutils(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif // luaosutils_hpp
