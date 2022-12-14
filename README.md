# luaosutils

Luaosutils is an ad-hoc collection of utility functions that access operating system features. It is written in C++ to be used with the Lua language. While the build projects are designed so that it is a simple drop-in to the Lua implementation running on Finale, it could be used with any Lua implementation if someone wants to take the time to develop the build process.

# Installation

The `release` directory contains two files:

- `luaosutils.dll` (for Windows)
- `luaosutils.dylib` (for macOS)

Choose the appropriate release for your operating system and place it in a folder where Lua can find it with `require`. For the _RGP Lua_ plugin running on Finale, a simple option is the same folder as the script that calls it.

If you are bundling this with a plugin suite for end users, you may need to build, sign, and notarize it yourself for it to be deployable without error messages on macOS.

# Functions

### download\_url

Downloads the contents of a url to a Lua string. The URL can be text or binary. The Lua string acts as a data buffer for the download and can be subsequently saved as a file (binary or text).
The function uses the HTTPS protocol. On Windows, HTTPS protocol is explicitly required in the code. On macOS, requiring HTTPS protocol is the default user setting.

|Input Type|Description|
|----------|-----------|
|string|The url to download.|
|function|The function to call when the download completes.|

|Output Type|Description|
|-----------|-----------|
|session|If non-nil, the callback function will be called. If nil, there was an error and it will not be called.|

The call is an asynchronous call. With this function you return control to Finale and leave its Lua state open with `finenv.RetainLuaState = true` or a dialog box. The completion function then finishes whatever needs to be done while running in the background. The completion function does not run in a separate thread, so you cannot wait on this function to complete directly within your script. You could, however, open a modal dialog box and allow it to wait for the completion function. A more user-friendly option would be a modeless dialog, because modeless dialogs do not block the user from completing other tasks. Be mindful of how long your script runs when running in the background.

You must keep a reference to the session until the callback is called. It will be aborted if the session variable goes out of scope and is garbage-collected.

The callback function has the following parameters.

|Input Type|Description|
|----------|-----------|
|boolean|Success or failure|
|string|The downloaded data if success. An error message or `nil` if failure.|


Example:

```lua
local osutils = require('luaosutils')

function callback(download_successful, urlcontents)
   if download_successful then
       local fileout = io.open(finenv.RunningLuaFolderPath().."/myfile.zip", "wb")
       fileout:write(urlcontents)
       fileout:close()
   end
   finenv.RetainLuaState = false
end

-- use a global to guarantee that it stays in scope in the callback
g_session = osutils.download_url("https://mysite.com/myfile.zip", callback)

finenv.RetainLuaState = true
```

The test folder contains [`test-luaosutil.lua`](https://github.com/finale-lua/luaosutils/blob/main/test/test-luaosutil.lua). This shows a working example that downloads the Google Mail icon to the folder where the script is running.

### download\_url\_sync

Downloads a url synchronously. You supply a timeout, and the function fails if the timeout expires. The timeout cannot be less than zero. Do not use synchronous calls except for very small files where you can limit the timeout to 1 or 2 seconds. Synchronous calls block Finale's user interface.

|Input Type|Description|
|----------|-----------|
|string|The url to download.|
|number|The timeout value in seconds. (May be fractional.)|


|Output Type|Description|
|----------|-----------|
|boolean|Success or failure|
|string|The downloaded data if success. An error message or `nil` if failure.|

Example:

```lua
local osutils = require('luaosutils')

local download_successful, urlcontents = osutils.download_url_sync("https://mysite.com/myfile.zip", 5)

if download_successful then
    local fileout = io.open(finenv.RunningLuaFolderPath().."/myfile.zip", "wb")
    fileout:write(urlcontents)
    fileout:close()
end
```

