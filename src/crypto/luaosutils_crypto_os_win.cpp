//
//  luaosutils_crypto_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 10/31/23.
//
#include <string>
#include <fstream>

//#include <wincrypt.h>
#include <bcrypt.h>

#include "crypto/luaosutils_crypto_os.h"
#include "crypto/luaosutils_crypto_utils.h"

namespace luaosutils
{

std::string calc_file_hash(const std::string& filePath)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file) return ""; // throw std::runtime_error("Error opening file");

   // Read the entire file into a buffer
   std::string buffer(std::istreambuf_iterator<char>(file), {});
   std::string retval;

   // Initialize variables
   BCRYPT_ALG_HANDLE hAlg = NULL;
   BCRYPT_HASH_HANDLE hHash = NULL;
   DWORD hashSize = 0;
   DWORD resultSize = 0;
   NTSTATUS status = 0;

   try
   {
      // Open an algorithm handle
      if (!BCRYPT_SUCCESS(status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA512_ALGORITHM, NULL, 0)))
         throw std::runtime_error("Failed to open algorithm handle.");
      // Determine the hash size
      if (!BCRYPT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hashSize), sizeof(hashSize), &resultSize, 0)))
         throw std::runtime_error("Failed to retrieve hash size.");
      encryptBuffer hashResult(hashSize);
      // Create a hash object
      if (!BCRYPT_SUCCESS(status = BCryptCreateHash(hAlg, &hHash, NULL, 0, NULL, 0, 0)))
         throw std::runtime_error("Failed to create hash object.");
      // Hash the input data
      if (!BCRYPT_SUCCESS(status = BCryptHashData(hHash, reinterpret_cast<PUCHAR>(const_cast<char*>(buffer.data())), static_cast<ULONG>(buffer.size()), 0)))
         throw std::runtime_error("Failed to hash the data.");
      // Finalize the hash
      if (!BCRYPT_SUCCESS(status = BCryptFinishHash(hHash, hashResult.data(), static_cast<ULONG>(hashResult.size()), 0)))
         throw std::runtime_error("Failed to finalize the hash.");
      retval = buffer2HexString(hashResult);
   }
   catch (std::exception&)
   {
      retval = std::string();
      // fall-thru to catchall return
   }
   // Clean up resources
   if (hHash) BCryptDestroyHash(hHash);
   if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
   return retval;
}

encryptBuffer calc_crypto_key(const encryptBuffer& seedValue, const encryptBuffer& salt)
{
   // Allocate memory for the key using a unique_ptr
   encryptBuffer key(cryptoKeyLength);
   const uint8_t* pSalt = salt.size() ? salt.data() : nullptr;
   
   // stoopid WinAPI can't take const salt, so effing copy it
   encryptBuffer salty = salt;
   uint8_t* pSalty = salty.size() ? salty.data() : nullptr;
   
   // Open a handle to the RNG algorithm
   BCRYPT_ALG_HANDLE rgnHandle = NULL;
   bool success = false;
   if (BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&rgnHandle, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG)))
   {
      // Generate the key using PBKDF2 with SHA-256
      NTSTATUS status =  BCryptDeriveKeyPBKDF2(rgnHandle, reinterpret_cast<PUCHAR>(seedValue.data()), static_cast<ULONG>(seedValue.size()),
                            reinterpret_cast<const PUCHAR>(pSalty), static_cast<ULONG>(salty.size()), kIterations, key.data(), keyLength, 0);
      if (!BCRYPT_SUCCESS(status))
      {
#ifdef _DEBUG
         // Get the error message
         DWORD error_code = GetLastError();
         LPSTR error_message = nullptr;
         FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        nullptr,
                        error_code,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        reinterpret_cast<LPSTR>(&error_message),
                        0,
                        nullptr);

         // Print the error message
         std::string msg = error_message;
//         std::cout << "Failed to derive key. Error code: " << error_code << ", message: " << error_message << std::endl;

         // Free the error message buffer
         LocalFree(error_message);
#endif
      }
      else
         success = true;
   }
   // Close the RNG handle
   if (rgnHandle) BCryptCloseAlgorithmProvider(rgnHandle, 0);
   if (! success) return encryptBuffer();

   return key;
}

encryptBuffer encrypt(const encryptBuffer& key, const std::string& plaintext, encryptBuffer& iv)
{
   encryptBuffer encryptedData;

   BCRYPT_ALG_HANDLE hAlgorithm = NULL;
   BCRYPT_KEY_HANDLE hKey = NULL;

   try
   {
      // Open a provider handle
      if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0)))
         throw std::runtime_error("Failed to open cryptographic provider");
      // Create a hash object for the key
      if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(hAlgorithm, &hKey, NULL, 0, const_cast<BYTE*>(key.data()), static_cast<ULONG>(key.size()), 0)))
         throw std::runtime_error("Failed to create hash object");
      // set chaining mode
      std::wstring mode = BCRYPT_CHAIN_MODE_CBC;
      BYTE* ptr = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(mode.data()));
      ULONG size = static_cast<ULONG>(sizeof(wchar_t) * (mode.size() + 1));
      if (!BCRYPT_SUCCESS(BCryptSetProperty(hAlgorithm, BCRYPT_CHAINING_MODE, ptr, size, 0)))
         throw std::runtime_error("Failed to set chain mode property");
      // generate iv
      ULONG block_len = 0;
      ULONG res = 0;
      if (!BCRYPT_SUCCESS(BCryptGetProperty(hAlgorithm, BCRYPT_BLOCK_LENGTH, reinterpret_cast<BYTE*>(&block_len), sizeof(block_len), &res, 0)))
         throw std::runtime_error("Failed to get block length property");
      iv = getSomeSalt(block_len);
      /* BCryptEncrypt modifies iv parameter, so we need to make copy */
      encryptBuffer iv_copy = iv;
      // Determine the size of the encrypted data
      ULONG dwDataLen = static_cast<ULONG>(data.length());
      ULONG dwResultLen = 0;
      if (!BCRYPT_SUCCESS(BCryptEncrypt(hKey, const_cast<BYTE*>(reinterpret_cast<const BYTE*>(data.data())), dwDataLen, NULL,
               iv_copy.data(), static_cast<ULONG>(iv_copy.size()), NULL, 0, &dwResultLen, BCRYPT_BLOCK_PADDING)))
         throw std::runtime_error("Failed to determine size of encrypted data");
      // Resize the encryptedData buffer
      encryptedData.resize(dwResultLen);
      // Encrypt the data
      if (!BCRYPT_SUCCESS(BCryptEncrypt(hKey, const_cast<BYTE*>(reinterpret_cast<const BYTE*>(data.data())), dwDataLen, NULL,
               iv_copy.data(), static_cast<ULONG>(iv_copy.size()), encryptedData.data(), dwResultLen, &dwResultLen, BCRYPT_BLOCK_PADDING)))
         throw std::runtime_error("Failed to encrypt data");
   }
   catch (std::runtime_error&)
   {
      encryptedData = encryptBuffer();
      // fall-thru to catch-all
   }

   // Clean up resources
   if (hKey != NULL)
      BCryptDestroyKey(hKey);
   if (hAlgorithm != NULL)
      BCryptCloseAlgorithmProvider(hAlgorithm, 0);

   return encryptedData;
}

std::string decrypt(const encryptBuffer& key, const encryptBuffer& cyphertext, const encryptBuffer& iv)
{
   std::string decryptedData;
   BCRYPT_ALG_HANDLE hAlgorithm = NULL;
   BCRYPT_KEY_HANDLE hKey = NULL;

   if (!iv.size())
      return decryptString_deprecated(key, encryptedData);

   try
   {
      // Open a provider handle
      if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0)))
         throw std::runtime_error("Failed to open cryptographic provider");
      // Create a hash object for the key
      if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(hAlgorithm, &hKey, NULL, 0, const_cast<BYTE*>(key.data()), static_cast<ULONG>(key.size()), 0)))
         throw std::runtime_error("Failed to create hash object");
      // set chaining mode
      std::wstring mode = BCRYPT_CHAIN_MODE_CBC;
      BYTE* ptr = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(mode.data()));
      ULONG size = static_cast<ULONG>(sizeof(wchar_t) * (mode.size() + 1));
      if (!BCRYPT_SUCCESS(BCryptSetProperty(hAlgorithm, BCRYPT_CHAINING_MODE, ptr, size, 0)))
         throw std::runtime_error("Failed to set chain mode property");
      /* BCryptEncrypt modifies iv parameter, so we need to make copy */
      encryptBuffer iv_copy = iv;
      // Determine the size of the decrypted data
      ULONG dwDataLen = static_cast<ULONG>(encryptedData.size());
      ULONG dwResultLen = 0;
      if (!BCRYPT_SUCCESS(BCryptDecrypt(hKey, const_cast<BYTE*>(reinterpret_cast<const BYTE*>(encryptedData.data())), dwDataLen, NULL,
               iv_copy.data(), static_cast<ULONG>(iv_copy.size()), NULL, 0, &dwResultLen, BCRYPT_BLOCK_PADDING)))
         throw std::runtime_error("Failed to determine size of decrypted data");
      // Resize the encryptedData buffer
      decryptedData.resize(dwResultLen);
      // Encrypt the data
      if (!BCRYPT_SUCCESS(BCryptDecrypt(hKey, const_cast<BYTE*>(reinterpret_cast<const BYTE*>(encryptedData.data())), dwDataLen, NULL,
               iv_copy.data(), static_cast<ULONG>(iv_copy.size()), reinterpret_cast<BYTE*>(decryptedData.data()), dwResultLen, &dwResultLen, BCRYPT_BLOCK_PADDING)))
         throw std::runtime_error("Failed to decrypt data");
      // Resize the encryptedData buffer
      decryptedData.resize(dwResultLen);
   }
   catch (std::runtime_error&)
   {
      decryptedData = std::string();
      // fall-thru to catch-all
   }

   // Clean up resources
   if (hKey != NULL)
      BCryptDestroyKey(hKey);
   if (hAlgorithm != NULL)
      BCryptCloseAlgorithmProvider(hAlgorithm, 0);

   return decryptedData;
}

} //namespace
