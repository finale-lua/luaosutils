//
//  luaosutils_crypto_os.mm
//  luaosutils
//
//  Created by Robert Patterson on 10/31/23.
//
#include <string>
#include <fstream>

#include <CommonCrypto/CommonCrypto.h>

#include "crypto/luaosutils_crypto_os.h"
#include "crypto/luaosutils_crypto_utils.h"


namespace luaosutils
{

std::string calc_file_hash(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file) return ""; // throw std::runtime_error("Error opening file");

   // Compute the SHA-512 hash of the file
   CC_SHA512_CTX context;
   CC_SHA512_Init(&context);
   const int bufsize = 4096;
   char buf[bufsize];
   while (file.read(buf, bufsize))
   {
      CC_SHA512_Update(&context, buf, bufsize);
   }
   CC_SHA512_Update(&context, buf, static_cast<CC_LONG>(file.gcount()));
   unsigned char hash[CC_SHA512_DIGEST_LENGTH];
   CC_SHA512_Final(hash, &context);
   
   return buffer2HexString(hash, CC_SHA512_DIGEST_LENGTH);
}

encryptBuffer calc_crypto_key(const encryptBuffer& seedValue, const encryptBuffer& salt)
{
   // Allocate memory for the key using a unique_ptr
   encryptBuffer key(cryptoKeyLength);
   const uint8_t* pSalt = salt.size() ? salt.data() : nullptr;
   
   // Generate the key using CommonCrypto
   CCKeyDerivationPBKDF(
                        kCCPBKDF2,                    // Algorithm
                        reinterpret_cast<const char*>(seedValue.data()), // Password
                        seedValue.size(),             // Password length
                        pSalt,                        // Salt (null defaults to all zeros)
                        salt.size(),                  // Salt length
                        kCCPRFHmacAlgSHA256,          // PRF (Hash function)
                        cryptoKeyIterations,          // Number of iterations
                        key.data(),                   // Derived key buffer
                        key.size()                    // Desired key length
                        );

   return key;
}
   
} //namespace
