//
//  luaosutils.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/9/2022.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include "luaosutils.hpp"

static const luaL_Reg funcs[] = {
   // currently no top-level functions
   {NULL, NULL} // sentinel
};

static int luaopen_luaosutils (lua_State *L, bool restricted) {
   /* export functions (and leave namespace table on top of stack) */
#if LUA_VERSION_NUM <= 501
   luaL_openlib(L, "luaosutils", funcs, 0);
#else
   luaL_newlib(L, funcs);
#endif
   /* add nested tables */
   luaosutils_internet_create(L, restricted);
   luaosutils_menu_create(L, restricted);
   luaosutils_process_create(L, restricted);
   luaosutils_text_create(L, restricted);
   /* make version string available to scripts */
   lua_pushstring(L, "_VERSION");
   lua_pushstring(L, LUAOSUTILS_VERSION);
   lua_rawset(L, -3);
   return 1;
}

int luaopen_luaosutils (lua_State *L)
{
   return luaopen_luaosutils(L, false);
}

int luaopen_luaosutils_restricted (lua_State *L)
{
   return luaopen_luaosutils(L, true);
}
