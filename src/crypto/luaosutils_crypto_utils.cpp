//
//  luaosutils_crypto_utils.cpp
//  luaosutils
//
//  Created by Robert Patterson on 10/31/23.
//

#include <string>
#include <sstream>
#include <random>
#include <vector>
#include <iomanip>
#include <ios>

#include "crypto/luaosutils_crypto_utils.h"

namespace luaosutils
{

std::string buffer2HexString(const uint8_t* buffer, const size_t size)
{
   std::stringstream ss;
   for (int i = 0; i < size; i++)
   {
      ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(buffer[i]);
   }
   return ss.str();
}

std::string buffer2HexString(const encryptBuffer& buffer)
{
   return buffer2HexString(buffer.data(), buffer.size());
}

encryptBuffer hexString2Buffer(const std::string& hexString)
{
   encryptBuffer buffer;
   buffer.reserve(hexString.size() / 2);
   
   for (size_t i = 0; i < hexString.size(); i += 2)
   {
      std::string byteString = hexString.substr(i, 2);
      try
      {
         uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
         buffer.push_back(byte);
      }
      catch (const std::invalid_argument&)
      {
         // skip invalid characters.
      }
   }
   
   return buffer;
}

encryptBuffer calc_randomized_data(int size)
{
   std::random_device rd; // obtain a random seed from the operating system
   std::mt19937 eng(rd()); // seed the generator
   std::uniform_int_distribution<int> distr(size <= 0 ? 32 : size,size <= 0 ? 96 : size); // define the range
   const int saltLength = distr(eng); // generate a random salt length between 32 and 96 bytes

   std::random_device rng; // create a cryptographically secure RNG
   encryptBuffer salt(saltLength); // create a vector to hold the salt value
   std::generate_n(salt.data(), salt.size(), std::ref(rng)); // generate the salt value
   return salt;
}

}
