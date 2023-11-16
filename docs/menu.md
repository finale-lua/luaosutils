# The 'menu' namespace

The `menu` namespace provides os-independent functions for manipulating menu items. For Finale this is particularly useful in the Plug-Ins menu, since by default Finale places all the plugins in a flat menu structure. These functions allow a script (executing at startup) to rearrange the plugin menus into a more usable menu tree.

The Windows and macOS operating systems treat menus slightly differently. In macOS, the top-level menu is associated with the application, whereas in Windows any window can have a top-level menu. Finale for Windows runs in an MDI Client window, and for Lua on Finale it is recommended to use the function `finenv.GetFinaleMainWindow()` to get it. This function (available starting in RGP Lua 0.66) returns the MDI Client Window handle in Windows or `nil` in macOS.

These functions use the following os-specific types. They appear in Lua as opaque userdata items. Lua scripts should only use them to pass to the menu functions in this library.

|Type|Description|
|----|-----------|
|menu_handle|os-assigned value that provides access to the menu|
|window_handle|handle (`HWND`) of a window containing a top-level menu (Windows) or `nil` (macOS)|

Here is an example script that uses the menu functions to move Finale's Music Spacing options to a top-level menu called "Spacing". (See `test/test-menubuild-luaosutil.lua` in this repository.)

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local main_window = finenv.GetFinaleMainWindow()
local menu_bar = menu.get_top_level_menu(main_window)
local spacing_menu, spacing_index = menu.find_item(menu_bar, "Apply Note Spacing to")
local new_spacing_menu = menu.insert_submenu("Spacing",  menu_bar, 4)
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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)

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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
if rgp_lua_menu then
    -- rgp_lua_menu is the menu that contains the menu item for RGP Lua.
end
```

### menu.get\_item\_command\_id

Returns the command-id of the specified menu item or `nil` if none.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The 0-based index of the submenu item.|

|Output Type|Description|
|----------|-----------|
|number|Command-id of the submenu or `nil` if none.|

On Windows this function returns the command-id. On macOS it returns the menu item's tag value. Finale uses the tag value to mimic the behavior of the Windows command-id. This function may be less useful if `luaosutils` is running on macOS in an environment other than Lua on Finale.

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua")
if rgp_lua_menu then
    local command_id = menu.get_item_command_id(rgp_lua_menu, index)
    finenv.UI():ExecuteOSMenuCommand(command_id) -- opens the RGP Lua configuration window
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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local number_of_items = menu.get_item_count(rgp_lua_menu)
end
```

### menu.get\_item\_submenu

Returns the submenu of the specified menu item, if it exists.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The 0-based index of the submenu item.|

|Output Type|Description|
|----------|-----------|
|menu_handle|Handle to the submenu or `nil` if none.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local main_menu, index = menu.find_item(menu_bar, "Plug-ins")
if main_menu then
    local plugins_menu = menu.get_item_submenu(main_menu, index)
end
```

### menu.get\_item\_text

Returns the text of the specified menu item.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The 0-based index of the menu item.|

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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local item_3_text = menu.get_item_text(rgp_lua_menu, 3)
end
```

### menu.get\_item\_type

Returns the type of the specified menu item.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the menu.|
|number|The 0-based index of the submenu item.|

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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local main_menu, index = menu.find_item(menu_bar, "Plug-ins")
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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
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
|(number)|The 0-based index where to insert the separator. If omitted, the separator is appended at the end.|

|Output Type|Description|
|----------|-----------|
|number|The 0-based index of the new separator or `nil` if error.|

Example:

```lua
local osutils = require('luaosutils')
local menu = osutils.menu

 -- Specify the minimum 0-based index of Finale's Plug-Ins menu in the top-level application menu.
local min_search_index = 6

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
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
|(number)|The 0-based index where to insert the new submenu. If omitted, the submenu is appended at the end.|

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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local submenu, submenu_index = menu.insert_submenu("Articulations", rgp_lua_menu, 3)
end
```

### menu.move\_item\*

Moves a menu item from one menu location to another.

|Input Type|Description|
|----------|-----------|
|menu_handle|Handle to the source menu.|
|number|The 0-based index of the source menu item.|
|menu_handle|Handle to the target menu.|
|(number)|Optional 0-based index of the target menu item. If omitted, the item is appended to the end of the menu.|

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
local rgp_lua_menu, index = menu.find_item(finale_menu, "RGP Lua...", min_search_index)
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
|number|The 0-based index of the menu item.|
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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
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

local menu_bar = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local rgp_lua_menu, index = menu.find_item(menu_bar, "RGP Lua...", min_search_index)
if rgp_lua_menu then
    local success = menu.set_title(rgp_lua_menu, finenv.GetFinaleMainWindow(), "Lua on Finale...")
end
```
