function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils menu test"
end

    
require('mobdebug').start() -- for ZeroBrane Studio debugging


local osutils = require('luaosutils')
local menu = osutils.menu

local main_window = finenv.GetFinaleMainWindow()

for k, v in pairsbykeys(menu) do
    print(tostring(k), tostring(v))
end

local function show_menu(menu_handle, index)
    print(menu.get_title(menu_handle, main_window), "count = "..menu.get_item_count(menu_handle), "index = "..tostring(index))
    for x = 0, menu.get_item_count(menu_handle)-1 do
        print("", menu.get_item_text(menu_handle, x))
    end
end

show_menu(menu.get_top_level_menu(main_window), 0)

local my_menu, index = menu.find_item(main_window, "Keep Octave Transposition in Concert Pitch")

menu.set_title(my_menu, main_window, menu.get_title(my_menu, main_window).." nya nya")
menu.set_item_text(my_menu, index, menu.get_item_text(my_menu, index).." bthht")

show_menu(my_menu, index)

--finenv.UI():AlertInfo(tostring(my_menu).." "..index, "")
