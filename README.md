# luaosutils

Luaosutils is an ad-hoc collection of utility functions that access operating system features. It is written in C++ to be used with the Lua language. While the build projects are designed so that it is a simple drop-in to the Lua implementation running on Finale, it could be used with any Lua implementation if someone wants to take the time to develop the build process.

# Installation

The `release` directory contains two files:

- `luaosutils.dll` (for Windows)
- `luaosutils.dylib` (for macOS)

Choose the appropriate release for your operating system and place it in a folder where Lua can find it with `require`. The _RGP Lua_ plugin running on Finale embeds a version of `luaosutils`, and no additional installation is necessary to use the embedded version. However, if you wish to use an external version with the plugin, a simple option is to place `luaosutils.dll` (Windows) or `luaosutils.dylib` (macOS) in the same folder as the script that `requires` it.

If you are bundling `luaosutils` externally with a plugin suite for end users, you may need to build, sign, and notarize it yourself for it to be deployable without error messages on macOS.

# Functions

\*Items marked with an asterisk are not available in restricted mode. You can load a restricted verision of the library as follows:

```lua
local osutils = require('luaosutils.restricted')
```

This prevents the script from doing things such as changing application menus, executing external code, or sending post requests to websites. The restricted mode is primarily useful to environments where `luaosutils` is embedded in the environment. The host app can determine if it trusts the code and either make the full or the restricted version of `luaosutils` available as is appropriate.

## The 'internet' namespace

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

With synchronous calls, you supply a timeout, and the function fails if the timeout expires. The timeout cannot be less than zero. Do not use synchronous calls except for very small replies where you can limit the timeout to 1 or 2 seconds. Synchronous calls block Finale's user interface.

Synchronous function names have a `_sync` prefix.

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

The test folder contains [`test-luaosutil.lua`](https://github.com/finale-lua/luaosutils/blob/main/test/test-luaosutil.lua). This shows a working example that downloads the Google Mail icon to the folder where the script is running.

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
local host = internet.server_name("https://mysite.com") -- return "mysite.com"
```

## The 'menu' namespace

The `menu` namespace provides os-independent functions for manipulating menu items. For Finale this is particularly useful in the Plug-Ins menu, since by default Finale places all the plugins in a flat menu structure. These functions allow a script (executing at startup) to rearrange the plugin menus into a more usable menu tree.

The Windows and macOS operating systems treat menus slightly differently. In macOS, the top-level menu is associated with the application, whereas in Windows any window can have a top-level menu. Finale for Windows runs in an MDI Client window, and for Lua on Finale it is recommended to use the function `finenv.GetFinaleMainWindow()` to get it. This function (available starting in RGP Lua 0.66) returns the MDI Client Window handle in Windows or `nil` in macOS.

These functions use the following os-specific types. They appear in Lua as opaque userdata items. Lua scripts should only use them to pass to the menu functions in this library.

|Type|Description|
|----|-----------|
|menu_handle|os-assigned value that provides acces to the menu|
|window_handle|handle (`HWND`) of a window containing a top-level menu (Windows) or `nil` (macOS)|

Here is an example script that uses the menu functions to move Finale's Music Spacing options to a top-level menu called "Spacing". (See `test/test-menubuild-luaosutil.lua` in this repository.)

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local main_window = finenv.GetFinaleMainWindow()
local spacing_menu, spacing_index = menu.find_item(main_window, "Apply Note Spacing to")
local new_spacing_menu = menu.insert_submenu("Spacing",  menu.get_top_level_menu(main_window), 4)
for i = 0, menu.get_item_count(spacing_menu)-1 do
    local move_result = menu.move_item(spacing_menu, 0, new_spacing_menu)
    print(move_result)
end
local del_result = menu.delete_submenu(spacing_menu, main_window)
print(del_result)
```

### menu.delete\_submenu\*

Deletes the specified submenu item from its parent menu. The submenu must contain zero menu items or it will not be deleted.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle of the submenu to be deleted.|
|window_handle|The window with the menu to search (Windows) or `nil` (macOS).|

|Output Type|Description|
|----------|-----------|
|boolean|True if the submenu was deleted.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)

-- add code the moves all items out of rgp_lua_menu

-- now delete the submenu that contained "RGP Lua..." if it no longer contains items
menu.delete_submenu(rgp_lua_menu, finenv.GetFinaleMainWindow())
```

### menu.find\_item

Searches a menu for an item whose text matches the input text and returns the enclosing menu if it is found. The function searches any submenus as well. To search all menus, pass in the value returned by `menu.get_top_level_menu`.

|Input Type|Description|
|----------|-----------|
|menu_handle|The menu from which to start searching.|
|string|The text to search for encoded in utf8.|
|(number)|Optional 0-based index that specifies the starting top-level menu from which to search. If omitted, the entire top-level menu is searched.|

|Output Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu that contains the item or `nil` if not found.|
|number|The 0-based index of the found item.|

The Windows version of this function skips "&" characters in the menu item text when doing comparisons. This allows you to supply the same text on either Windows or macOS platforms and find the menu options and ignore the "&" characters that designate keyboard shortcuts on Windows.

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    -- rgp_lua_menu is the menu that contains the menu item for RGP Lua.
end
```

### menu.get\_item\_command\_id

Returns the command-id of the specified menu item or `nil` if none.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The index of the submenu item.|

|Output Type|Description|
|----------|-----------|
|number|Command-id of the submenu or `nil` if none.|

On Windows this function returns the command-id. On macOS it returns the menu item's tag value. Finale uses the tag value to mimic the behavior of the Windows command-id. This function may be less useful if `luaosutils` is running on macOS in an environment other than Lua on Finale.

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua")
if main_menu then
    local command_id = menu.get_item_command_id(rgp_lua_menu, index)
end
```

### menu.get\_item\_count

Returns the number of menu items in the specified menu. It includes divider items.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|

|Output Type|Description|
|----------|-----------|
|number|The number of items in the menu.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local number_of_items = menu.get_item_count(rgp_lua_menu)
end
```

### menu.get\_item\_submenu

Returns the text of the specified menu item.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The index of the submenu item.|

|Output Type|Description|
|----------|-----------|
|menu_handle|Handle to the submenu or `nil` if none.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local main_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "Plug-ins")
if main_menu then
    local plugins_menu = menu.get_item_submenu(main_menu, index)
end
```

### menu.get\_item\_text

Returns the text of the specified menu item.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The index of the menu item.|

|Output Type|Description|
|----------|-----------|
|string|The text of menu item (UTF-8 encoding).|

On Windows, the text includes the `&` character if there is keyboard shortcut. 

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local item_3_text = menu.get_item_text(rgp_lua_menu, 3)
end
```

### menu.get\_item\_type

Returns the text of the specified menu item.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The index of the submenu item.|

|Output Type|Description|
|----------|-----------|
|number|A value representing the menu item type (see table of values)|

The following are the possible values returned by this function.

|Constant Name|Value|Description|
|-------------|-----|-----------|
|ITEMTYPE_INVALID|-1|The input was not a valid menu type.|
|ITEMTYPE_COMMAND|0|Regular menu command item. This is the default if no others are applicable.|
|ITEMTYPE_SUBMENU|1|Submenu item.|
|ITEMTYPE_SEPARATOR|2|Separator item.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local main_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "Plug-ins")
if main_menu then
    local plugins_menu = menu.get_item_submenu(main_menu, index)
    for item = 0, menu.get_item_count(plugins_menu)-1 do
        local item_type = menu.get_item_type(plugins_menu, item)
        if item_type == menu.ITEMTYPE_SUBMENU then
            -- do something with submenu
        end
    end
end
```

### menu.get\_title

Returns the title of the specified menu.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|window_handle|Handle to the window containing the menu (`nil` on macOS).|

|Output Type|Description|
|----------|-----------|
|string|The title of menu (UTF-8 encoding).|

On Windows, the title includes the `&` character if it has keyboard shortcut. 

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local menu_title = menu.get_title(rgp_lua_menu, finenv.GetFinaleMainWindow())
end
```

### menu.get\_top\_level\_menu

Returns a handle to the top-level menu for the specified window (Windows) or the currently running application (macOS).

|Input Type|Description|
|----------|-----------|
|window_handle|The window with the menu to search (Windows) or `nil` (macOS).|

|Output Type|Description|
|----------|-----------|
|menu_handle|Handle to the top-level menu or `nil` if not found.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local finale_menu = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
```

### menu.insert\_separator\*

Inserts a menu separator at the specified index.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|(number)|The index where to insert the separator. If omitted, the separator is appended at the end.|

|Output Type|Description|
|----------|-----------|
|number|The 0-based index of the new separator or `nil` if error.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local submenu, submenu_index = menu.insert_submenu("Articulations", rgp_lua_menu, 3)
end
```

### menu.insert\_submenu\*

Inserts a new submenu at the specified index.

|Input Type|Description|
|----------|-----------|
|string|Title of the new submenu encoded UTF-8.|
|menu_handle|Handle of the menu in which to insert the new submenu.|
|(number)|The index where to insert the new submenu. If omitted, the submenu is appended at the end.|

|Output Type|Description|
|----------|-----------|
|menu_handle|Handle to the new submenu or `nil` if not found.|
|number|The 0-based index of the new submenu.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local submenu, submenu_index = menu.insert_submenu("Articulations", rgp_lua_menu, 3)
end
```

### menu.move\_item\*

Moves a menu item from one menu location to another.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the source menu.|
|number|The index of the source menu item.|
|menu_handle|Handle to the target menu.|
|(number)|Optional index of the target menu item. If omitted, the item is appended to the end of the menu.|

|Output Type|Description|
|----------|-----------|
|boolean|True if the menu item was moved.|
|number|The new 0-based index of the item in the target menu.|

This function can only move base menu items. It cannot move submenus. If you pass in an item for a submenu it returns false. To move a submenu, create the new submenu and then move all the items for the submenu individually.

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local finale_menu = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
	 -- move the RGP Lua configuration menu option to the end of the Finale menu.
    menu.move_item(rgp_lua_menu, index, finale_menu)
end
```

### menu.redraw\*

Redraws the menu bar. This function does nothing on macOS.

|Input Type|Description|
|----------|-----------|
|window_handle|Handle to the window containing the menu (`nil` on macOS).|

|Output Type|Description|
|----------|-----------|
|none||

Always call this function after modifying the menus, especially if you have modified the top menu. It it is sometimes not necessary, especially when running at startup, but it does not hurt anything.

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

menu.redraw(finenv.GetFinaleMainWindow())
```

### menu.set\_item\_text\*

Changes the text of the specified menu item to the new value.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The index of the menu item.|
|string|The new text of menu item (UTF-8 encoding).|

|Output Type|Description|
|----------|-----------|
|boolean|True if the menu item text was changed.|

On Windows, the text includes the `&` character if there is keyboard shortcut. 

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local success = menu.set_item_text(rgp_lua_menu, index, "Lua on Finale...")
end
```

### menu.set\_title\*

Returns the title of the specified menu.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|window_handle|Handle to the window containing the menu (`nil` on macOS).|
|string|The new title of menu (UTF-8 encoding).|

|Output Type|Description|
|----------|-----------|
|boolean|True if the menu title was changed.|

On Windows, the title includes the `&` character if it has keyboard shortcut. 

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local rgp_lua_menu, index = menu.find_item(finenv.GetFinaleMainWindow(), "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local success = menu.set_title(rgp_lua_menu, finenv.GetFinaleMainWindow(), "Lua on Finale...")
end
```

## The 'process' namespace

The `process` namespace offers functions to launch a separate process. The advantage of these APIs over the standard Lua APIs is that the process is launched *silently*. No console window appears on either macOS or Windows.

The optional folder path for the working directory must be a fully qualified path name. Do not enclose this string in outer quote marks even if the path name contains spaces. If you do, Windows will not recognize it as a path name, and the function will fail. On macOS the functions do not fail, but the outer quote marks are not necessary either. For example, you can directly pass `finenv.RunningLuaFolderPath()` directly on either operating system. Do not enclose it in quotes, even if the running lua path contains spaces.

### process.execute\*

Executes a process with the input command line and waits for it to complete. It captures any text the process sends to `stdio` and returns it in a string.

|Input Type|Description|
|----------|-----------|
|string|The command line to execute encoded in UTF-8.|
|(string)|Optional folder path to set as the working directory for the process (also UTF-8).|

|Output Type|Description|
|----------|-----------|
|string|Output from the executed process or `nil` if there was an error.|

On macOS the returned string is encoded in UTF-8.

On Windows, the string is returned unmodified from the output of the process. Lua scripts must handle any character encoding themselves. It may be helpful to use the `&` pipe character to prepend your command with a call to `chcp 65001`. That sets the active code page to UTF-8. As long as the program you run emits text in the active code page, your returned text will be encoded UTF-8.

Example:

```lua
local osutils = require('luaosutils')
local process = osutils.process

if finenv.UI():IsOnWindows() then
    local listing = process.execute('cmd /c chcp 65001 & REG QUERY \"HKLM\\Software\\Microsoft\" /reg:32')
    -- listing is now a string containing a listing of the specified registry key.
end
```

### process.launch\*

Launches a process with the input command line and returns immediately.

|Input Type|Description|
|----------|-----------|
|string|The command line to execute encoded in UTF-8.|
|(string)|Optional folder path to set as the working directory for the process (also UTF-8).|

|Output Type|Description|
|----------|-----------|
|boolean|True if process was successfully launched.|

Note that successfully launching the process does not mean that the command line pointed to a valid program. If your program does not exist, `process.launch` may still return `true`, because it does not wait to see what the result of running the process is.

Example:

```lua
local osutils = require('luaosutils')
local process = osutils.process

if finenv.UI():IsOnMac() then
    -- launch Safari and return immediately
    local success = process.launch("open /Applications/Safari.app")
end
```

## The 'text' namespace

The text namespace helps with managing text encoding. Since macOS operates entirely in UTF-16, text encoding is primarily a concern on Windows. For this reason the encoding value inputs are Windows codepage numbers. However, the `text` namespace is fully platform-independent. For many common codepages the same inputs achieve the same outputs on both platforms.

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

# Version history

2.2.0

- Added `post` functions to the `internet` namespace, along with the option to specify html headers.
- Rename `download_url` functions as `get`, but maintain `download_url` functions as aliases.
- Windows version of `menu.find_item` now skips '&' on the search string as well as the menu item strings.
- Prebuilt binaries compiled with Lua 5.4
- Added an untrusted code variant that can be loaded with `require('luaosutils.restricted')`.
- Added `launch_website` and `server_name` to the `internet` namespace.

2.1.1

- Add explicit Objective-C memory management (macOS) to allow memory-safe embedding in RGP Lua.
- The `window_handle` is now a required parameter for the Windows version of `menu.redraw`.

2.1.0

- Refactored url download functions into `internet` namespace.
- Added `menu` namespace with APIs for rearranging menu options.
- Added `process` namespace with APIs for silent process launching and execution.
- Added `text` namespace with APIs for text encoding.

1.1.1

- Better windows error handling

1.1.0

- original release
