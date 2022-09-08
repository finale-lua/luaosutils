# luaosutils

Luaosutils is an ad-hoc collection of utility functions that access operating system features. It is written in C++ to be used with the Lua language. While the build projects are designed so that it is a simple drop-in to the Lua implementation running on Finale, it could be used with any Lua implementation if someone wants to take the time to develop the build process.

# Installation

The `release` directory contains two files:

- `luaosutils.dylib` (for macOS)
- `luaosutils.dll` (for Windows)

Choose the appropriate release for your operating system and place it in a folder where Lua can find it with `require`. For the _RGP Lua_ plugin running on Finale, a simple option is the same folder as the script that calls it.

If you are bundling this with a plugin suite for end users, you may need to build, sign, and notarize it yourself for it to be deployable without error messages on macOS.

# Functions

### download_url

Downloads the contents of a url to a Lua string. The URL can be text or binary. The Lua string acts as a data buffer for the download and can be subsequently saved as a file (binary or text).

|Input Type|Description|
|----------|-----------|
|string|The url to download.|
|function|The function to call when the download completes.|

|Output Type|Description|
|-----------|-----------|
|boolean|If true, the callback function will be called. If false, there was an error and it will not be called.|

The call is an asynchronous call. You script could wait in a while loop for it to finish.
But a better option would be to  return control to Finale and leave its Lua state open with `finenv.RetainLuaState = true`. The completion function could then finish whatever needs to be done running in the background. (However, the completion function does not run in a separate thread, so be mindful of how long your script runs when running in the background.)

The callback function has the following parameters.

|Input Type|Description|
|----------|-----------|
|boolean|Success or failure|
|string|The downloaded string if success. An error message or `nil` if failure.|


Example:

```lua
local osutils = require('luaosutils')

function callback(download_successful, urlcontents)
   if download_successful then
   	   -- do something with the url contents in result here
   end
end

local download_started = osutils.download_url("https://mysite.com/myfile.txt", callback)

finenv.RetainLuaState = true
```


