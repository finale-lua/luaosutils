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

#define LUAOSUTILS_VERSION "Luaosutils 2.1.0"

#define MAC_OS       1         /* Macintosh operating system */
#define WINDOWS      2         /* Microsoft Windows (MS-DOS) */
#define UNKNOWN_OS   -1

#if defined(__GNUC__)
#define OPERATING_SYSTEM MAC_OS
#elif defined(_MSC_VER)
#define OPERATING_SYSTEM WINDOWS
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

#include "luaosutils_export.h"

#ifndef __OBJC__
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif // __GNUC__
#include "LuaBridge/LuaBridge.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // __GNUC__
#endif // __OBJC__

//ToDo: __download_callback should be in url/luaosutils_url.cpp
using __download_callback = std::function<void (bool, const std::string&)>;

//utility functions

#define DIM(a) (sizeof(a)/sizeof(a[0]))

#ifndef __OBJC__
template<typename T>
T __get_lua_parameter(lua_State* L, int param_number, int expected_type, std::optional<T> default_value = std::nullopt)
{
   const int type = lua_type(L, param_number);
   const bool is_nil = (type == LUA_TNIL || type == LUA_TNONE);
   if (type != expected_type && (!default_value.has_value() || !is_nil))
   {
      const char* expected_type_name = lua_typename(L, expected_type);
      const char* actual_type_name = lua_typename(L, type);
      luaL_error(L, "param %d expected %s, got %s", param_number, expected_type_name, actual_type_name);
   }
   if (is_nil)
   {
      if (default_value.has_value())
         return default_value.value();
      luaL_error(L, "param %d must supply default value for nil", param_number);
   }
   if constexpr(std::is_same<T, luabridge::LuaRef>::value)
   {
      return luabridge::Stack<luabridge::LuaRef>::get(L, param_number);
   }
   else if constexpr (std::is_convertible<T, void*>::value)
   {
      T ptr = reinterpret_cast<T>(lua_touserdata(L, param_number));
      return ptr;
   }
   else
   {
      luabridge::LuaRef ref = luabridge::Stack<luabridge::LuaRef>::get(L, param_number);
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

inline void __add_constant(lua_State *L, const char* const_name, int value, int table_index)
{
   lua_pushstring(L, const_name);
   lua_pushinteger(L, value);
   lua_rawset(L, table_index);
}

void luaosutils_internet_create(lua_State *L);
void luaosutils_menu_create(lua_State *L);
void luaosutils_process_create(lua_State *L);
void luaosutils_text_create(lua_State *L);

#endif // luaosutils_hpp
