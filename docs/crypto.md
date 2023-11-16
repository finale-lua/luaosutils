# The 'crypto' namespace

This namespace provides access to OS-level cryptography routines. Most of the inputs and outputs are Lua strings, but some contain binary data and some contain hexadecimal digits (ASCII) representing binary data. The library also provides routines to convert between these two formats efficiently.

## Functions

- [`calc_crypto_key`](#cryptocalc_crypto_key) : Uses PBKDF2 with SHA-256 to create a key appropriate for encryption and decryption.
- [`calc_file_hash`](#cryptocalc_file_hash) : Computes the SHA-512 hash for a file and returns it in a character string of hexadecimal digits.
- [`calc_randomized_data`](#cryptocalc_randomized_data) : Returns a binary string of randomly initialized bytes.
- [`conv_bin_to_chars`](#cryptoconv_bin_to_chars) : Converts a binary string to hexadecimal digits.
- [`conv_chars_to_bin`](#cryptoconv_chars_to_bin) : Converts hexadecimal digits to a binary string.
- [`decrypt`](#cryptodecrypt) : Uses AES encryption to decrypt the input cyphertext.
- [`encrypt`](#cryptoencrypt) : Uses AES encryption to encrypt the input plaintext.

### crypto.conv\_bin\_to\_chars

Converts a binary string to the equivalent string of hexadecimal digits.

|Input Type|Description|
|----------|-----------|
|string|A string of binary values between 0 and 255.|

|Output Type|Description|
|----------|-----------|
|string|A string of pairs of hexadecimal digits that correspond to the binary data.|

```lua
local bin_string = "\x53\x4d\x3a\xf4"
local chars_string = crypto.conv_bin_to_chars(bin_string)
print (chars_string) -- prints "534d3af4"
```

### crypto.conv\_chars\_to\_bin

Converts a string of hexadecimal digits to the equivalent string of binary values. Any characters that are not hexadecimal digits are skipped.

|Input Type|Description|
|----------|-----------|
|string|A string of pairs of hexadecimal digits ('0'-'9' and 'a'-'f').|

|Output Type|Description|
|----------|-----------|
|string|A string of binary values corresponding to each pair of hexadecimal digits.|

```lua
local chars_string = "534d3af4"
local bin_string = crypto.conv_chars_to_bin(chars_string)
print (bin_string)
```

### crypto.calc\_randomized\_data

Returns a binary string of randomly initialized bytes. This is routine is useful both to create key salt and to create iv (see below).

|Input Type|Description|
|----------|-----------|
|(integer)|The optional number of randomized bytes to return. If omitted, it returns a random number of bytes between 32 and 96. |

|Output Type|Description|
|----------|-----------|
|string|A string of randomly initialized binary values.|


```lua
-- this is an appropriate way to create key salt for encryption:
local key_salt = crypto.calc_randomized_data()
```

### crypto.calc\_file\_hash

Computes the SHA-512 hash for a file and returns it in a character string of pairs of hexadecimal digits.

|Input Type|Description|
|----------|-----------|
|string|The file path of the file for which to compute the hash.|


|Output Type|Description|
|----------|-----------|
|string|The SHA-512 hash represented as pairs of hexadecimal digits ('0'-'9' and 'a'-'f').|


```lua
local file_path = "mypath/mypath.text"
local hash = crypto.conv_chars_to_bin(file_path)
```

### crypto.calc\_crypto\_key

Uses PBKDF2 with SHA-256 to create a key appropriate for encryption and decryption. It is important to choose a key seed that is random and difficult to guess. A random password generator is one effective approach.

|Input Type|Description|
|----------|-----------|
|string|The seed (as binary bytes). This corresponds to a passphrase or other sequence of secret characters.|
|string|Key salt (as binary bytes). Calculate new key salt every time you encrypt with the seed.|

|Output Type|Description|
|-----------|-----------|
|string|The key represented as binary bytes.|

```lua
local seed = "my secret password" -- choose something less guessable than this
local salt = crypto.calc_randomized_data() -- you will need this to decrypt
local key = crypto.calc_crypto_key(seed, salt)
```

### crypto.encrypt

Uses AES encryption to encrypt the input plaintext. After encryption you will have three separate binary strings that are needed in order to decrypt:

- cyphertext (the encrypted text)
- key salt (used to recreate the encryption key)
- iv (used to initialize the encryption buffer)

You can safely store these in the clear anywhere that makes sense for your application. The seed value should be stored securely and separately from the program. One approach might be to put it in an environment variable when your program runs.

|Input Type|Description|
|----------|-----------|
|string|The encyption key (binary string)|
|string|The plaintext to encrypt|


|Output Type|Description|
|-----------|-----------|
|string|The encrypted cyphertext (binary string)|
|string|The iv used to initialize the encryption buffer (binary string)|

```lua
local plaintext = "The plaintext to encrypt"
local seed = "my secret password" -- choose something less guessable than this
local salt = crypto.calc_randomized_data() -- you will need this to decrypt
local key = crypto.calc_crypto_key(seed, salt)
local cyphertext, iv = crypto.encrypt(key, plaintext)

-- to decrypt the cyphertext, you will need the salt and the iv, both of which
-- you can safely store in the clear alongside the cyphertext.
```

### crypto.decrypt

Uses AES encryption to decrypt the input cyphertext. In addition to the cyphertext, you
need the key salt and iv that were generated as part of the encryption process.


|Input Type|Description|
|----------|-----------|
|string|The encyption key (binary string)|
|string|The cyphertext to decrypt (binary string)|
|string|The iv from the call to `encrypt` (binary string)|

|Output Type|Description|
|-----------|-----------|
|string|The decrypted plaintext.|

```lua
local cyphertext -- retrieved from wherever you stored it after encryption
local salt -- retrieved from wherever you stored it after encryption
local iv -- retrieved from wherever you stored it after encryption

local seed = "my secret password" -- one way to get this value might be an environment variable
local key = crypto.calc_crypto_key(seed, salt)
local plaintext = crypto.decrypt(key, cyphertext, iv)
```
