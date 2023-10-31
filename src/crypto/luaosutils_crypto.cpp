//
//  luaosutils_crypto.cpp
//  luaosutils
//
//  Created by Robert Patterson on 10/31/23.
//
#include "luaosutils.hpp"
#include "crypto/luaosutils_crypto_os.h"
#include "crypto/luaosutils_crypto_utils.h"

static int luaosutils_conv_bin_to_chars(lua_State* L)
{
   auto bin = get_lua_parameter<luaosutils::encryptBuffer>(L, 1, LUA_TSTRING);
   push_lua_return_value(L, luaosutils::buffer2HexString(bin));
   return 1;
}

static int luaosutils_conv_chars_to_bin(lua_State* L)
{
   auto chars = get_lua_parameter<std::string>(L, 1, LUA_TSTRING);
   push_lua_return_value(L, luaosutils::hexString2Buffer(chars));
   return 1;
}

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

static int luaosutils_crypto_encrypt(lua_State* L)
{
   auto key = get_lua_parameter<luaosutils::encryptBuffer>(L, 1, LUA_TSTRING);
   auto plaintext = get_lua_parameter<std::string>(L, 2, LUA_TSTRING);
   luaosutils::encryptBuffer iv;
   luaosutils::encryptBuffer result = luaosutils::encrypt(key, plaintext, iv);
   push_lua_return_value(L, result);
   push_lua_return_value(L, iv);
   return 2;
}

static int luaosutils_crypto_decrypt(lua_State* L)
{
   auto key = get_lua_parameter<luaosutils::encryptBuffer>(L, 1, LUA_TSTRING);
   auto cyphertext = get_lua_parameter<luaosutils::encryptBuffer>(L, 2, LUA_TSTRING);
   auto iv = get_lua_parameter<luaosutils::encryptBuffer>(L, 3, LUA_TSTRING);
   std::string result = luaosutils::decrypt(key, cyphertext, iv);
   push_lua_return_value(L, result);
   return 1;
}

static const luaL_Reg crypyo_utils[] = {
   {"conv_bin_to_chars",         luaosutils_conv_bin_to_chars},
   {"conv_chars_to_bin",         luaosutils_conv_chars_to_bin},
   {"calc_randomized_data",      luaosutils_crypto_calc_randomized_data},
   {"calc_file_hash",            luaosutils_crypto_calc_file_hash},
   {"calc_crypto_key",           luaosutils_crypto_calc_crypto_key},
   {"encrypt",                   luaosutils_crypto_encrypt},
   {"decrypt",                   luaosutils_crypto_decrypt},
   {NULL, NULL} // sentinel
};


void luaosutils_crypto_create(lua_State *L)
{
   lua_newtable(L);  // create nested table
   
   luaL_setfuncs(L, crypyo_utils, 0);   // add file methods to new metatable
   lua_setfield(L, -2, "crypto");       // add the nested table to the parent table with the name
}

