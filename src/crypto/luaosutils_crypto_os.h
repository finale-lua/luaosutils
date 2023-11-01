//
//  luaosutils_crypto_os.h
//  luaosutils
//
//  Created by Robert Patterson on 10/31/23.
//
#ifndef luaosutils_crypto_os_h
#define luaosutils_crypto_os_h

#include <string>

#include "crypto/luaosutils_crypto_utils.h"

namespace luaosutils
{

std::string calc_file_hash(const std::string& filePath);
encryptBuffer calc_crypto_key(const encryptBuffer& seedValue, const encryptBuffer& salt);
encryptBuffer encrypt(const encryptBuffer& key, const std::string& plaintext, encryptBuffer& iv);
std::string decrypt(const encryptBuffer& key, const encryptBuffer& cyphertext, const encryptBuffer& iv);

}

#endif /* luaosutils_crypto_os_h */
