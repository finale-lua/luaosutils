function plugindef()
    finaleplugin.RequireDocument = false
    finaleplugin.LoadLuaOSUtils = true
    return "aaa - luautils show menu tree"
end

    
require('mobdebug').start() -- for ZeroBrane Studio debugging


local osutils = require('luaosutils.restricted')
local menu = osutils.menu

local main_window = finenv.GetFinaleMainWindow()

local function disp_cmd(command_id)
    if type(command_id) ~= "number" then return "nil" end
    if finenv.UI():IsOnWindows() then
        return string.format("0x%x", command_id)
    end
    local b1 = string.char(bit32.band(bit32.rshift(command_id, 24), 0xFF))
    local b2 = string.char(bit32.band(bit32.rshift(command_id, 16), 0xFF))
    local b3 = string.char(bit32.band(bit32.rshift(command_id, 8), 0xFF))
    local b4 = string.char(bit32.band(command_id, 0xFF))
    return "'" .. b1 .. b2 .. b3 .. b4 .. "'"
end

local show_menu_tree
show_menu_tree = function(menu_handle, indent, command_id)
    indent = indent or ""
    print(indent..menu.get_title(menu_handle, main_window).." >", disp_cmd(command_id))
    for x = 0, menu.get_item_count(menu_handle)-1 do
        local command_id = menu.get_item_command_id(menu_handle, x)
        local new_indent = indent.."    "
        local item_type = menu.get_item_type(menu_handle, x)
        if item_type == menu.ITEMTYPE_SUBMENU then
            local submenu = menu.get_item_submenu(menu_handle, x)
            show_menu_tree(submenu, new_indent, command_id)
        elseif item_type == menu.ITEMTYPE_SEPARATOR then
            print(new_indent.."----------", disp_cmd(command_id))
        elseif item_type == menu.ITEMTYPE_INVALID then
            print(new_indent.."<invalid item>", disp_cmd(command_id))
        else
            print(new_indent..menu.get_item_text(menu_handle, x), disp_cmd(command_id))
        end
    end
    print(indent.."<")
end

show_menu_tree(menu.get_top_level_menu(main_window))
