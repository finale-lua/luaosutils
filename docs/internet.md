# The 'internet' namespace

This namespace provides functions to send `GET` or `POST` requests to web servers. The functions then return the full response in a Lua string. For asynchronous calls, the response is passed to a callback function.

##### Asynchronous calls

Asynchronous calls are the recommended option. They return a session variable that you maintain until the request completes. You return control to Finale and leave its Lua state open either with `finenv.RetainLuaState = true` or a dialog box or both. The completion function then finishes whatever needs to be done while running in the background.

The completion function does not run in a separate thread, so you cannot wait on the request to complete directly within your script. You could, however, open a modal dialog box and allow it to wait for the completion function. A more user-friendly option would be a modeless dialog, because modeless dialogs do not block the user from completing other tasks. Be mindful of how long your completion function runs when running in the background, because it blocks the UI.

You must keep a reference to the session until the callback is called. Your request is aborted if the session variable goes out of scope and is garbage-collected. Your request is also aborted if the Lua state that created it is closed.

The callback function has the following parameters.

|Input Type|Description|
|----------|-----------|
|boolean|Success or failure|
|string|The downloaded data if success. An error message or `nil` if failure.|


##### Synchronous calls

With synchronous calls, you supply a timeout, and the function fails if the timeout expires. The timeout cannot be less than zero. Do not use synchronous calls except for very small replies where you can limit the timeout to a few seconds. Synchronous calls block Finale's user interface.

Synchronous function names have a `_sync` suffix.

##### HTTPS required

These functions use the HTTPS protocol. On Windows, HTTPS protocol is explicitly required in the code. On macOS, requiring HTTPS protocol is the default user setting.

##### HTML headers

The functions all have an optional headers parameter that allows you to include HTML headers on the request message. These take the form of a table of key/value pairs (all strings).

Example:

```lua
local headers = {
            ["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/94.0.4606.81 Safari/537.36",
            ["Accept-Language"] = "en-US,en;q=0.9"
        }
```

### internet.get

Downloads the contents of a url to a Lua string using a `GET` request. The URL resource can be text or binary.

|Input Type|Description|
|----------|-----------|
|string|The url to download.|
|function|The callback function to call when the download completes.|
|(headers)|An optional table of html headers.|

|Output Type|Description|
|-----------|-----------|
|session|If nil, there was an error.|


Example:

```lua
local osutils = require('luaosutils')
local internet = osutils.internet

function callback(download_successful, urlcontents)
   if download_successful then
       local fileout = io.open(finenv.RunningLuaFolderPath().."/myfile.zip", "wb")
       fileout:write(urlcontents)
       fileout:close()
   end
   finenv.RetainLuaState = false
end

-- use a global to guarantee that it stays in scope in the callback
g_session = internet.get("https://mysite.com/myfile.zip", callback)

finenv.RetainLuaState = true
```

The test folder contains [`test-internet-luaosutil.lua`](https://github.com/finale-lua/luaosutils/blob/main/test/test-internet-luaosutil.lua). This shows a working example that downloads the Google Mail icon to the folder where the script is running.

This function is also aliased as `download_url` for backwards compatibility.

### internet.get\_sync

Downloads the contents of a url synchronously to a Lua string using a `GET` request. The URL resource can be text or binary.

|Input Type|Description|
|----------|-----------|
|string|The url to download.|
|number|The timeout value in seconds. (May be fractional.)|
|(headers)|An optional table of html headers.|


|Output Type|Description|
|----------|-----------|
|boolean|Success or failure|
|string|The downloaded data if success. An error message or `nil` if failure.|

Example:

```lua
local osutils = require('luaosutils')
local internet = osutils.internet

local download_successful, urlcontents = internet.get_sync("https://mysite.com/myfile.zip", 5)

if download_successful then
    local fileout = io.open(finenv.RunningLuaFolderPath().."/myfile.zip", "wb")
    fileout:write(urlcontents)
    fileout:close()
end
```

This function is also aliased as `download_url_sync` for backwards compatibility.

### internet.launch\_website

Launches the specified URL in the user's default web browser.

|Input Type|Description|
|----------|-----------|
|string|The url to launch.|

Example:


```lua
local osutils = require('luaosutils')
local internet = osutils.internet
internet.launch_website("https://mysite.com")
```

### internet.post

Post data to a url using a `POST` request and returns the response a Lua string. The data returned may be text or binary.

|Input Type|Description|
|----------|-----------|
|string|The url to send the request to.|
|string|The data to post.|
|function|The callback function to call when the download completes.|
|(headers)|An optional table of html headers.|

|Output Type|Description|
|-----------|-----------|
|session|If nil, there was an error.|


Example:

```lua
local osutils = require('luaosutils')
local internet = osutils.internet

function callback(download_successful, urlcontents)
   if download_successful then
       local fileout = io.open(finenv.RunningLuaFolderPath().."/myfile.json", "wb")
       fileout:write(urlcontents)
       fileout:close()
   end
   finenv.RetainLuaState = false
end

local post_data = "<your post data> (maybe JSON?)"

-- use a global to guarantee that it stays in scope in the callback
g_session = internet.post("https://mysite.com", post_data, callback)

finenv.RetainLuaState = true
```

### internet.post\_sync

Post data synchronously to a url using a `POST` request and returns the response a Lua string. The data returned may be text or binary.

|Input Type|Description|
|----------|-----------|
|string|The url to download.|
|string|The data to post.|
|number|The timeout value in seconds. (May be fractional.)|
|(headers)|An optional table of html headers.|


|Output Type|Description|
|----------|-----------|
|boolean|Success or failure|
|string|The downloaded data if success. An error message or `nil` if failure.|

Example:

```lua
local osutils = require('luaosutils')
local internet = osutils.internet

local post_data = "<your post data> (maybe JSON?)"

local download_successful, urlcontents = internet.post_sync("https://mysite.com", post_data , 5)

if download_successful then
    local fileout = io.open(finenv.RunningLuaFolderPath().."/myfile.json", "wb")
    fileout:write(urlcontents)
    fileout:close()
end
```

### internet.close\_session

Cancels and closes the session for a pending asynchronous request. You callback will not be called after calling this function. It is not necessary to call this if your script is ending,
but you might call it if you are closing a window that has the callback procedure while
retaining your Lua session.

|Input Type|Description|
|----------|-----------|
|session|May be nil, and then the function does nothing.|

|Output Type|Description|
|----------|-----------|
|nil|May be used to clear the script's session variable (see example).|

```lua
local osutils = require('luaosutils')
local internet = osutils.internet

local post_data = "<your post data> (maybe JSON?)"
local session = internet.post_sync("https://mysite.com", post_data , function(success, data) end)

-- no need to check for nil here first.
-- after calling the function, session is assigned to nil.
session = internet.close_session(session)
```


### internet.server\_name

Returns the servername contained within the specified URL.

|Input Type|Description|
|----------|-----------|
|string|The url to examine.|

|Output Type|Description|
|----------|-----------|
|string|The name of the host contained within the url.|

Example:

```lua
local osutils = require('luaosutils')
local internet = osutils.internet
local host = internet.server_name("https://mysite.com") -- returns "mysite.com"
```
