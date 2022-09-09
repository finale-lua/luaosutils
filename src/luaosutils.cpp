//
//  luaosutils.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/9/2022.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include <string>
#include <exception>

#include "luaosutils.hpp"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif
#include "LuaBridge/LuaBridge.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/** \brief demonstrates a simple mathematical calculation that adds 1 to an input value.
 *
 * stack position 1: the url to download
 * stack position 2: a reference to a lua function to call on completion
 * \return download status (success or failuer)
 */
static int luaosutils_download_url (lua_State *L)
{
   bool retval = false;
   std::string urlString = luabridge::Stack<std::string>::get(L, 1);
   luabridge::LuaRef callback = luabridge::Stack<luabridge::LuaRef>::get(L, 2);
   if (! callback.isFunction())
      luaL_error(L, "Function download_url expects a callback function in the second argument.");

   luabridge::Stack<bool>::push(L, retval);
   return 1;
}

static const luaL_Reg luaosutils[] = {
   {"download_url",     luaosutils_download_url},
   {NULL, NULL} // sentinel
};

int luaopen_luaosutils (lua_State *L) {
#if LUA_VERSION_NUM <= 501
   luaL_openlib(L, "luaosutils", luaosutils, 0);
#else
   luaL_newlib(L, luaosutils);
#endif
   return 1;
}
