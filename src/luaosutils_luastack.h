//
//  luaosutils_luastack.h
//  luaosutils-static
//
//  Created by Robert Patterson on 3/31/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
//  Replaces luabridge::Stack. This get function is not intended for general use but
//  rather works in tandem with the checks in get_lua_parameter.

#ifndef luaosutils_luastack_h
#define luaosutils_luastack_h

#ifndef __OBJC__

#include <string>

#include "lua.hpp"
#include "crypto/luaosutils_crypto_utils.h"

template<typename T>
class LuaStack
{
   template<typename Arg, typename... Args>
   friend int push_lua_args(lua_State* L, Arg arg, Args... args);

public:
   LuaStack(lua_State* L) : L(L) {}
   
   void push(T value) {
      push_impl(value);
   }
   
   T get(int index) {
      return get_helper<T>::get(L, index);
   }

private:
   lua_State* L;
   
   template<typename U>
   struct get_helper {
      static U get(lua_State*, int) {
         // Default implementation throws an error
         static_assert(sizeof(U) == 0, "No specialized implementation for this type");
         return U();
      }
   };
   
   template<typename U>
   void push_impl(U) {
      static_assert(sizeof(U) == 0, "No specialized implementation for this type");
   }
   
   template<>
   struct get_helper<bool> {
      static bool get(lua_State* L, int index) {
         return lua_toboolean(L, index);
      }
   };
   
   template<>
   struct get_helper<int> {
      static int get(lua_State* L, int index) {
         if (lua_isfunction(L, index))
         {
            lua_pushvalue(L, index);
            return luaL_ref(L, LUA_REGISTRYINDEX);
         }
         return static_cast<int>(lua_tointeger(L, index));
      }
   };
   
   template<>
   struct get_helper<unsigned int> {
      static unsigned int get(lua_State* L, int index) {
         return static_cast<unsigned int>(lua_tointeger(L, index));
      }
   };

   template<>
   struct get_helper<long> {
      static long get(lua_State* L, int index) {
         return static_cast<long>(lua_tointeger(L, index));
      }
   };

   template<>
   struct get_helper<double> {
      static double get(lua_State* L, int index) {
         return lua_tonumber(L, index);
      }
   };
   
   template<>
   struct get_helper<std::string> {
      static std::string get(lua_State* L, int index) {
         const char* str = lua_tolstring(L, index, nullptr);
         if (str == nullptr) return "";
         const size_t len = lua_rawlen(L, index);
         return std::string(str, len);
      }
   };
   
   template<>
   struct get_helper<luaosutils::encryptBuffer> {
      static luaosutils::encryptBuffer get(lua_State* L, int index) {
         const uint8_t* val = (const uint8_t*)lua_tolstring(L, index, nullptr);
         if (val == nullptr) return luaosutils::encryptBuffer();
         const size_t len = lua_rawlen(L, index);
         return luaosutils::encryptBuffer(val, val + len);
      }
   };

   void push_impl(bool value) {
      lua_pushboolean(L, value);
   }
   
   void push_impl(int value) {
      lua_pushinteger(L, value);
   }
   
   void push_impl(unsigned int value) {
      lua_pushinteger(L, value);
   }

   void push_impl(long value) {
      lua_pushinteger(L, value);
   }

   void push_impl(double value) {
      lua_pushnumber(L, value);
   }
   
   void push_impl(const std::string& value) {
      lua_pushlstring(L, value.data(), value.size());
   }
   
   void push_impl(const luaosutils::encryptBuffer& value) {
      lua_pushlstring(L, (const char*)value.data(), value.size());
   }
};

// Base case for the recursive call
inline int push_lua_args(lua_State*)
{
   return 0; // No more arguments to push
}

// Helper function to push multiple arguments onto the stack
template<typename Arg, typename... Args>
int push_lua_args(lua_State* L, Arg arg, Args... args) {
   LuaStack<Arg>(L).push_impl(arg);
   return 1 + push_lua_args(L, args...);
}

template<typename T>
T get_lua_parameter(lua_State* L, int param_number, int expected_type, std::optional<T> default_value = std::nullopt, const char* metatableKey = nullptr)
{
   const int type = lua_type(L, param_number);
   if (type == LUA_TNIL || type == LUA_TNONE)
   {
      if (default_value.has_value())
         return default_value.value();
   }
   if (type != expected_type)
   {
      const char* expected_type_name = metatableKey ? metatableKey : lua_typename(L, expected_type);
      const char* actual_type_name = lua_typename(L, type);
      luaL_error(L, "param %d expected %s, got %s", param_number, expected_type_name, actual_type_name);
   }
   if constexpr (std::is_convertible<T, void*>::value)
   {
      T ptr = (metatableKey)
               ? reinterpret_cast<T>(luaL_checkudata(L, param_number, metatableKey))
               : reinterpret_cast<T>(lua_touserdata(L, param_number));
      return ptr;
   }
   else
   {
      return LuaStack<T>(L).get(param_number);
   }
}

template<typename T>
void push_lua_return_value(lua_State* L, const T& retval)
{
   if constexpr (std::is_convertible<T, void*>::value)
   {
      if (! retval)
         lua_pushnil(L);
      else
         lua_pushlightuserdata(L, retval);
   }
   else
      LuaStack<T>(L).push(retval);
}

#endif // __OBJC__
#endif // luaosutils_luastack_h
