function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils menu test"
end

    
require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')
local menu = osutils.menu

local main_window = finenv.GetFinaleMainWindow()

local my_menu, index = menu.find_item(main_window, "Keep Octave Transposition in Concert Pitch")

finenv.UI():AlertInfo(tostring(my_menu).." "..index, "")
