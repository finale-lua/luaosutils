function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils show menu tree"
end

    
require('mobdebug').start() -- for ZeroBrane Studio debugging


local osutils = require('luaosutils')
local menu = osutils.menu

local main_window = finenv.GetFinaleMainWindow()

local show_menu_tree
show_menu_tree = function(menu_handle, indent)
    indent = indent or ""
    print(indent..menu.get_title(menu_handle, main_window).." >")
    for x = 0, menu.get_item_count(menu_handle)-1 do
        local new_indent = indent.."    "
        local item_type = menu.get_item_type(menu_handle, x)
        if item_type == menu.ITEMTYPE_SUBMENU then
            local submenu = menu.get_item_submenu(menu_handle, x)
            show_menu_tree(submenu, new_indent)
        elseif item_type == menu.ITEMTYPE_SEPARATOR then
            print(new_indent.."----------")
        elseif item_type == menu.ITEMTYPE_INVALID then
            print(new_indent.."<invalid item>")
        else
            print(new_indent..menu.get_item_text(menu_handle, x))
        end
    end
    print(indent.."<")
end

show_menu_tree(menu.get_top_level_menu(main_window))
