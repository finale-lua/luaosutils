//
//  luaosutils_crypto_utils.h
//  luaosutils
//
//  Created by Robert Patterson on 10/31/23.
//
#ifndef luaosutils_crypto_utils_h
#define luaosutils_crypto_utils_h

#include <string>
#include <vector>

constexpr int cryptoKeyLength = 32;
constexpr long cryptoKeyIterations = 10327;

namespace luaosutils
{

using encryptBuffer = std::vector<uint8_t>;
std::string buffer2HexString(const uint8_t* buffer, const size_t size);
std::string buffer2HexString(const luaosutils::encryptBuffer& buffer);
encryptBuffer hexString2Buffer(const std::string& hexString);

encryptBuffer calc_randomized_data(int size = -1);

}
#endif /* luaosutils_crypto_utils_h */
