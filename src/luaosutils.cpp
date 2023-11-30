//
//  luaosutils.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/9/2022.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include "luaosutils.hpp"

lua_CFunction luaosutils_errfunc_callback = nullptr;

// note: these globals only affect
constexpr static uint32_t kRestrictHttps = 0x0001;
constexpr static uint32_t kRestrictMenus = 0x0002;
constexpr static uint32_t kRestrictExternal = 0x0004;
static uint32_t g_restrictedOptions = 0; //this is always reset to zero after initialization

static const luaL_Reg funcs[] = {
   // currently no top-level functions
   {NULL, NULL} // sentinel
};

static int luaopen_luaosutils(lua_State *L, bool restricted, bool restrictMenus)
{
   /* export functions (and leave namespace table on top of stack) */
#if LUA_VERSION_NUM <= 501
   luaL_openlib(L, "luaosutils", funcs, 0);
#else
   luaL_newlib(L, funcs);
#endif
   /* add nested tables */
   luaosutils_crypto_create(L);
   luaosutils_internet_create(L, (g_restrictedOptions & kRestrictHttps) != 0);
   luaosutils_menu_create(L, (g_restrictedOptions & kRestrictMenus) != 0);
   luaosutils_process_create(L, (g_restrictedOptions & kRestrictExternal) != 0);
   luaosutils_text_create(L);
   /* make version string available to scripts */
   lua_pushstring(L, "_VERSION");
   lua_pushstring(L, LUAOSUTILS_VERSION);
   lua_rawset(L, -3);
   g_restrictedOptions = 0;
   return 1;
}

void luaosutils_set_permissions(const bool restrictHttps, const bool restrictMenus, const bool restrictExternal)
{
   g_restrictedOptions = 0;
   if (restrictHttps)
      g_restrictedOptions |= kRestrictHttps;
   if (restrictMenus)
      g_restrictedOptions |= kRestrictMenus;
   if (restrictExternal)
      g_restrictedOptions |= kRestrictExternal;
}

int luaopen_luaosutils(lua_State *L)
{
   return luaopen_luaosutils(L, false, false);
}

int luaopen_luaosutils_restricted(lua_State *L)
{
   luaosutils_set_permissions(false, true, true);
   return luaopen_luaosutils(L, true, true);
}
