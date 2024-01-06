# The 'internet' namespace

- [`cancel_session`](#internetcancel_session) : Cancels a pending asynchronous request and closes its session.
- [`get`](#internetget) : Sends HTTPS `GET` command and retrieves the response. (Asynchronous)
- [`get_sync`](#internetget_sync): Sends HTTPS `GET` command and retrieves the response. (Synchronous)
- [`launch_website`](#internetlaunch_website) : Launches a URL in the default browser.
- [`post`](#internetpost) : Sends HTTPS `POST` command and retrieves the response. (Asynchronous)
- [`post_sync`](#internetpost_sync): Sends HTTPS `POST` command and retrieves the response. (Synchronous)
- [`report_errors`](#internetreport_errors) : Sets whether the session should report errors to the user.
- [`server_name`](#internetserver_name) : Extracts the servername from a URL.
- [`url_escape`](#interneturl_escape) : Replaces non-transmissible characters with `%` codes.

This namespace provides functions to send `GET` or `POST` requests to web servers. The functions then return the full response in a Lua string. For asynchronous calls, the response is passed to a callback function.

##### Asynchronous calls

Asynchronous calls are the recommended option. They return a session variable that you maintain until the request completes. You return control to Finale and leave its Lua state open either with `finenv.RetainLuaState = true` or a dialog box or both. The completion function then finishes whatever needs to be done while running in the background.

The completion function does not run in a separate thread, so you cannot wait on the request to complete directly within your script. You *can* call `process.run_main_thread` in a loop while waiting, and your callback will eventually be called. However, since the user interface remains blocked, there is normally no advantage to doing this over a synchronous call. In fact, there is a potential disadvantage because synchronous calls unblock as soon as they finish whereas `run_event_loop` blocks for the full specified timeout period.

If you are launching calls from a dialog box, you should definitely use asynchronous calls. Your dialog box keeps the script alive while yielding control back to the operating system to enable UI response and callbacks. Be mindful of how long your completion function runs when running in the background, because it blocks the UI.

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

### internet.cancel\_session*

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
local session = internet.post("https://mysite.com", post_data , function(success, data) end)

-- no need to check for nil here first.
-- after calling the function, session is assigned to nil.
session = internet.cancel_session(session)
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

### internet.report\_errors*

By default, errors that occur inside the async callback function are reported to the user in a popup dialog box. You can disable that behavior with this function. Use with caution, since disabling the error reporting could cause the script to fail silently. Generally, you should only set this if your script is known to be hosted by another script that is monitoring the return value.

|Input Type|Description|
|----------|-----------|
|session|May be nil, and then the function does nothing.|
|boolean|True means report errors. False means do not report errors.|

|Output Type|Description|
|----------|-----------|
|none||

```lua
local osutils = require('luaosutils')
local internet = osutils.internet

local post_data = "<your post data> (maybe JSON?)"
local session = internet.post("https://mysite.com", post_data , function(success, data) end)

-- do not report errors
internet.report_errors(session, false)
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


### internet.url\_escape

Returns a string with characters converted to percent codes as needed for URLs. Most such characters are encoded, including "%" and "#", so you should not pass in a string that has already been percent-encoded. Since the function uses OS-specific APIs, there are slight platform differences in encoding. Notably:

- "?" is percent-encoded on Windows but not on macOS.
- "/" is percent-encoded on Windows but not on macOS.

The best way to use this function is to build a URL component by component. You can use the function to encode a component but the caller should append the control characters.

|Input Type|Description|
|----------|-----------|
|string|The string to process.|

|Output Type|Description|
|----------|-----------|
|string|Version of the string with non-url characters converted to codes or an empty string if error.|

Example:

```lua
local osutils = require('luaosutils')
local internet = osutils.internet
local raw_string = "my string with spaces"
local url_string = internet.url_escape(raw_string)
print(url_string) -- prints "my%20string%20with%20spaces"
```
