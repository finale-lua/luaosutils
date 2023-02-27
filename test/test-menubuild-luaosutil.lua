function plugindef()
    finaleplugin.RequireDocument = false
    finaleplugin.LoadLuaOSUtils = true
    return "aaa - luautils build menu test"
end

    
--require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')
local menu = osutils.menu

local top_level_menu = menu.get_top_level_menu(finenv.GetFinaleMainWindow())
local spacing_menu, spacing_index = menu.find_item(top_level_menu, "Apply Note Spacing to")
local new_spacing_menu = menu.insert_submenu("Spacing",  menu.get_top_level_menu(main_window), 4)
for i = 0, menu.get_item_count(spacing_menu)-1 do
    local move_result, move_index = menu.move_item(spacing_menu, 0, new_spacing_menu)
    print(move_result, move_index)
end
local del_result = menu.delete_submenu(spacing_menu, finenv.GetFinaleMainWindow())
print(del_result)

menu.redraw(main_window)

--finenv.UI():AlertInfo(tostring(my_menu).." "..index, "")
