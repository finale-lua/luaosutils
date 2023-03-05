//
//  luaosutils_text.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/27/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#include "luaosutils.hpp"
#include "text/luaosutils_text_os.h"

#if OPERATING_SYSTEM == WINDOWS
#include <windows.h>
#else
#define CP_UTF8 65001
#endif

static int luaosutils_text_convert_encoding(lua_State *L)
{
   auto text = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   auto fromCodepage = get_lua_parameter<unsigned int>(L, 2, LUA_TNUMBER);
   auto toCodepage = get_lua_parameter<unsigned int>(L, 3, LUA_TNUMBER, CP_UTF8);

   if (!text.size())
   {
      push_lua_return_value(L, text);
      return 1;
   }

   // Encode even when fromCodepage == toCodepage
   // This allows a script to find out if fromCodepage is a valid encoding for the string.
   if (fromCodepage && toCodepage)
   {
      std::string output;
      const bool result = text_convert_encoding(text, fromCodepage, output, toCodepage);
      if (result)
         push_lua_return_value(L, output);
      else
         lua_pushnil(L);
   }
   else
      lua_pushnil(L);

   return 1;
}

static const luaL_Reg text_utils[] = {
   {"convert_encoding",       luaosutils_text_convert_encoding},
   {NULL, NULL} // sentinel
};

void luaosutils_text_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   luaL_setfuncs(L, text_utils, 0);   // add file methods to new metatable
   lua_setfield(L, -2, "text");       // add the nested table to the parent table with the name
}
