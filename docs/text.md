# The 'text' namespace

The text namespace helps with managing text encoding. Since macOS operates entirely in UTF-16, text encoding is primarily a concern on Windows. For this reason the encoding value inputs are Windows codepage numbers. However, the `text` namespace is fully platform-independent. For many common codepages the same inputs achieve the same outputs on both platforms.

## Functions

- [`convert_encoding`](#textconvert_encoding) : Converts encoding from one codepage to another.
- [`get_default_codepage`](#textget_default_codepage) : Gets the current default codepage.
- [`get_utf8_codepage`](#textget_utf8_codepage) : Gets the `utf8` codepage value, so your script need not hardcode it.

### text.convert_encoding

|Input Type|Description|
|----------|-----------|
|string|The string to convert.|
|number|The Windows codepage number in which the string is currently encoded.|
|(number)|The Windows codepage number to which to encode the string (UTF-8 if omitted).|

|Output Type|Description|
|----------|-----------|
|string|String encoded in the target codepage or `nil` if error.|

If you pass the same value for the current and target codepages, `text.convert_encoding` still encodes the string. For example, you could use this to test if a string of unknown encoding is a valid UTF-8 string. A non-nil return value means it is a valid UTF-8 string.

Example:

```lua
local osutils = require('luaosutils')
local text = osutils.text

local my_string = "<some text>"

local utf8_string = text.convert_encoding(my_string, 65001, 65001) -- test if my_string is UTF-8
local w1252_string = text.convert_encoding(my_string, 65001, 1252) -- convert string from UTF-8 to Windows 1252
local utf8_string2 = text.convert_encoding(w1252_string, 1252) -- convert 1252 to UTF-8 (with omitted 3rd parameter).
```

### text.get\_default\_codepage

On Windows, returns the current ANSI codepage value (as returned by the `GetACP` WinAPI function). On macOS, returns the UTF-8 codepage value.

|Input Type|Description|
|----------|-----------|

|Output Type|Description|
|----------|-----------|
|number|The codepage value of the current default codepage or 0 if error.
|string|If the codepage value is 0, an error message that describes the error.|

Example:

```lua
local osutils = require('luaosutils')
local text = osutils.text

local default_codepage, error_msg = text.get_default_codepage() -- 1252 on many Windows systems
```

### text.get\_utf8\_codepage

Returns the codepage value of UTF-8 using OS-level API calls. This obviates the need for calling code to hardcode it.

```lua
local osutils = require('luaosutils')
local text = osutils.text

local utf8_codepage = text.get_utf8_codepage() -- almost certainly will be 65001
```
