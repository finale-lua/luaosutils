# luaosutils

Luaosutils is an ad-hoc collection of utility functions that access operating system features. It is written in C++ to be used with the Lua language. While the build projects are designed so that it is a simple drop-in to the Lua implementation running on Finale, it could be used with any Lua implementation if someone wants to take the time to develop the build process.

# Installation

The `release` directory contains two files:

- `luaosutils.dll` (for Windows)
- `luaosutils.dylib` (for macOS)

Choose the appropriate release for your operating system and place it in a folder where Lua can find it with `require`. For the _RGP Lua_ plugin running on Finale, a simple option is the same folder as the script that calls it.

If you are bundling this with a plugin suite for end users, you may need to build, sign, and notarize it yourself for it to be deployable without error messages on macOS.

# Functions

## The 'internet' namespace

### internet.download\_url

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
g_session = internet.download_url("https://mysite.com/myfile.zip", callback)

finenv.RetainLuaState = true
```

The test folder contains [`test-luaosutil.lua`](https://github.com/finale-lua/luaosutils/blob/main/test/test-luaosutil.lua). This shows a working example that downloads the Google Mail icon to the folder where the script is running.

### internet.download\_url\_sync

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
local internet = osutils.internet

local download_successful, urlcontents = internet.download_url_sync("https://mysite.com/myfile.zip", 5)

if download_successful then
    local fileout = io.open(finenv.RunningLuaFolderPath().."/myfile.zip", "wb")
    fileout:write(urlcontents)
    fileout:close()
end
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

### menu.delete\_submenu

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
|(window_handle)|Handle to the window containing the menu (may be omitted on macOS).|

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

### menu.insert\_separator

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

### menu.insert\_submenu

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

### menu.move\_item

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

### menu.redraw

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

## The 'process' namespace

The `process` namespace offers functions to launch a separate process. The advantage of these APIs over the standard Lua APIs is that the process is launched *silently*. No console window appears on either macOS or Windows.

### process.execute

Executes a process with the input command line and waits for it to complete. It captures any text the process sends to `stdio` and returns it in a string.

|Input Type|Description|
|----------|-----------|
|string|The command line to execute.|


|Output Type|Description|
|----------|-----------|
|string|Output from the executed process or `nil` if there was an error.|

Example:

```lua
local osutils = require('luaosutils')
local process = osutils.process

if finenv.UI():IsOnWindows() then
    local listing = process.execute('cmd /c dir "C:/Program Files"')
    -- listing is now a string containing the directory listing of C:/Program Files.
end
```

### process.launch

Launches a process with the input command line and returns immediately.

|Input Type|Description|
|----------|-----------|
|string|The command line to execute.|


|Output Type|Description|
|----------|-----------|
|boolean|True if process was successfully launched.|

Example:

```lua
local osutils = require('luaosutils')
local process = osutils.process

if finenv.UI():IsOnMac() then
    -- launch Safari and return immediately
    local success = process.launch("open /Applications/Safari.app")
end
```

# Version history

2.1.0

- Refactored url download functions into `internet` namespace.
- Added `menu` namespace with APIs for rearranging menu options.
- Added `process` namespace with APIs for silent process launching and execution.

1.1.1

- Better windows error handling

1.1.0

- original release
	