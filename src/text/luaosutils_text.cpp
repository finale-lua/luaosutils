//
//  luaosutils_text.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/27/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#if OPERATING_SYSTEM == WINDOWS
#include <windows.h>
#endif

#include "luaosutils.hpp"
#include "text/luaosutils_text_os.h"

static int luaosutils_text_reencode(lua_State *L)
{
   std::string text = __get_lua_parameter(L, 1, std::string());
   unsigned int fromCodepage = __get_lua_parameter(L, 2, 0u);
   unsigned int toCodepage = __get_lua_parameter(L, 3, WINCODE(CP_UTF8) MACCODE(65001));

   if (!text.size())
   {
      __push_lua_return_value(L, text);
      return 1;
   }

   // Re-encode even when fromCodepage == toCodepage
   // This allows a script to find out if fromCodepage is a valid encoding for the string.
   if (fromCodepage && toCodepage)
   {
      std::string output;
      const bool result = __text_reencode(text, fromCodepage, output, toCodepage);
      if (result)
         __push_lua_return_value(L, output);
      else
         lua_pushnil(L);
   }
   else
      lua_pushnil(L);

   return 1;
}

static const luaL_Reg text_utils[] = {
   {"reencode",            luaosutils_text_reencode},
   {NULL, NULL} // sentinel
};

void luaosutils_text_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   luaL_setfuncs(L, text_utils, 0);   // add file methods to new metatable
   lua_setfield(L, -2, "text");       // add the nested table to the parent table with the name
}
