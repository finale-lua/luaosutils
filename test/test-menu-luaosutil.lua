function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils menu test"
end

    
require('mobdebug').start() -- for ZeroBrane Studio debugging


local osutils = require('luaosutils')
local menu = osutils.menu

for k, v in pairsbykeys(menu) do
    print(tostring(k), tostring(v))
end

local function show_menu(menu_handle, index)
    print(tostring(menu_handle), "count = "..menu.get_item_count(menu_handle), "index = "..tostring(index))
    for x = -1, menu.get_item_count(menu_handle)-1 do
        print("", menu.get_item_text(menu_handle, x))
    end
end

local main_window = finenv.GetFinaleMainWindow()

show_menu(menu.get_top_level_menu(main_window), 0)

local my_menu, index = menu.find_item(main_window, "Keep Octave Transposition in Concert Pitch")

show_menu(my_menu, index)

--finenv.UI():AlertInfo(tostring(my_menu).." "..index, "")
