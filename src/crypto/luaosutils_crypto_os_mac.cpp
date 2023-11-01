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

encryptBuffer encrypt(const encryptBuffer& key, const std::string& plaintext, encryptBuffer& iv)
{
   // Convert the plaintext from a string to a byte array
   const char* plaintextBytes = plaintext.c_str();
   const size_t plaintextLength = plaintext.length();
   
   // create the IV
   iv = calc_randomized_data(kCCBlockSizeAES128);
   
   // Allocate space for the encrypted data
   size_t encryptedLength = plaintextLength + kCCBlockSizeAES128;
   std::unique_ptr<char[]> encryptedBytes(new char[encryptedLength]);
   
   size_t actualEncryptedLength = 0;
   [[maybe_unused]]CCCryptorStatus
   result = CCCrypt(kCCEncrypt,
                    kCCAlgorithmAES,
                    kCCOptionPKCS7Padding,
                    key.data(),
                    key.size(),
                    iv.data(),
                    plaintextBytes,
                    plaintextLength,
                    encryptedBytes.get(),
                    encryptedLength,
                    &actualEncryptedLength);
   // Convert the encrypted byte array to a string and clean up
   encryptBuffer encryptedString(encryptedBytes.get(), encryptedBytes.get() + actualEncryptedLength);
   
   return encryptedString;
}

std::string decrypt(const encryptBuffer& key, const encryptBuffer& cyphertext, const encryptBuffer& iv)
{
   const char* encryptedBytes = reinterpret_cast<const char*>(cyphertext.data());
   const size_t encryptedLength = cyphertext.size();
   
   // Allocate space for the decrypted data
   size_t decryptedLength = encryptedLength + kCCBlockSizeAES128;
   std::unique_ptr<char[]> decryptedBytes(new char[decryptedLength]);
   
   size_t actualDecryptedLength = 0;
   [[maybe_unused]]CCCryptorStatus
   result = CCCrypt(kCCDecrypt,
                    kCCAlgorithmAES,
                    kCCOptionPKCS7Padding,
                    key.data(),
                    key.size(),
                    iv.size() ? iv.data() : nullptr,
                    encryptedBytes,
                    encryptedLength,
                    decryptedBytes.get(),
                    decryptedLength,
                    &actualDecryptedLength);
   // Convert the decrypted byte array to a string and clean up
   std::string decryptedString(decryptedBytes.get(), actualDecryptedLength);
   
   return decryptedString;
}

} //namespace
