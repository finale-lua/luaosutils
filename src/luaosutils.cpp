//
//  luaosutils.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/9/2022.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include "luaosutils.hpp"

bool luaosutils_trusted = true;

static const luaL_Reg funcs[] = {
   // currently no top-level functions
   {NULL, NULL} // sentinel
};

int luaopen_luaosutils (lua_State *L) {
   /* export functions (and leave namespace table on top of stack) */
#if LUA_VERSION_NUM <= 501
   luaL_openlib(L, "luaosutils", funcs, 0);
#else
   luaL_newlib(L, funcs);
#endif
   /* add nested tables */
   luaosutils_internet_create(L);
   luaosutils_menu_create(L);
   luaosutils_process_create(L);
   luaosutils_text_create(L);
   /* make version string available to scripts */
   lua_pushstring(L, "_VERSION");
   lua_pushstring(L, LUAOSUTILS_VERSION);
   lua_rawset(L, -3);
   return 1;
}
