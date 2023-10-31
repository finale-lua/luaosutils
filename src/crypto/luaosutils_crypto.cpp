//
//  luaosutils_crypto.cpp
//  luaosutils
//
//  Created by Robert Patterson on 10/31/23.
//
#include "luaosutils.hpp"
#include "crypto/luaosutils_crypto_os.h"
#include "crypto/luaosutils_crypto_utils.h"

static int luaosutils_crypto_calc_randomized_data(lua_State* L)
{
   auto size = get_lua_parameter<int>(L, 1, LUA_TNUMBER, -1);
   luaosutils::encryptBuffer result = luaosutils::calc_randomized_data(size);
   push_lua_return_value(L, result);
   return 1;
}

static int luaosutils_crypto_calc_file_hash(lua_State* L)
{
   auto pathString = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   std::string retval = luaosutils::calc_file_hash(pathString);
   LuaStack<std::string>(L).push(retval);
   return 1;
}

static int luaosutils_crypto_calc_crypto_key(lua_State* L)
{
   auto seed = get_lua_parameter<luaosutils::encryptBuffer>(L, 1, LUA_TSTRING);
   auto salt = get_lua_parameter<luaosutils::encryptBuffer>(L, 2, LUA_TSTRING);
   luaosutils::encryptBuffer result = luaosutils::calc_crypto_key(seed, salt);
   push_lua_return_value(L, result);
   return 1;
}

static const luaL_Reg crypyo_utils[] = {
   {"calc_randomized_data",      luaosutils_crypto_calc_randomized_data},
   {"calc_file_hash",            luaosutils_crypto_calc_file_hash},
   {"calc_crypto_key",           luaosutils_crypto_calc_crypto_key},
   {NULL, NULL} // sentinel
};


void luaosutils_crypto_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   luaL_setfuncs(L, crypyo_utils, 0);   // add file methods to new metatable
   lua_setfield(L, -2, "crypto");       // add the nested table to the parent table with the name
}

