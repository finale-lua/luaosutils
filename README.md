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

## The 'menu' namespace

The `menu` namespace provides os-independent functions for manipulating menu items. For Finale this is particularly useful in the Plug-Ins menu, since by default Finale places all the plugins in a flat menu structure. These functions allow a script (executing at startup) to rearrange the plugin menus into a more usable menu tree.

The Windows and macOS operating systems treat menus slightly differently. In macOS, the top-level menu is associated with the application, whereas in Windows any window can have a top-level menu. Finale for Windows runs in an MDI Client window, and for Lua on Finale it is recommended to use the function `finenv.GetFinaleMainWindow()` to get it. This function (available starting in RGP Lua 0.66) returns the MDI Client Window handle in Windows or `nil` in macOS.

These functions use the following os-specific types. These appear in Lua as opaque userdata items. Lua scripts should only use them to pass to other menu functions in this library.

|Type|Description|
|----|-----------|
|menu_handle|os-assigned value that provides acces to the menu|
|window_handle|handle (`HWND`) of a window containing a top-level menu (Windows) or `nil` (macOS)|

### menu.find\_item

Searches the currently running application's top-level menu for text that matches the input text and returns the enclosing menu if it is found.

|Input Type|Description|
|----------|-----------|
|window_handle|The window with the menu to search (Windows) or `nil` (macOS).|
|string|The text to search for encoded in utf8.|
|(number)|Optional 0-based index that specifies the starting top-level menu from which to search. If omitted, the entire top-level menu is searched.|

|Output Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu or `nil` if not found.|
|number|The 0-based index of the found item.|

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

### menu.get\_title

Returns the title of the specified menu.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|window_handle|Handle to the window containing the menu (may be omitted on macOS).|

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

### menu.set\_item\_text

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

### menu.set\_title

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

