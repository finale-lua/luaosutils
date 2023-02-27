function plugindef()
    finaleplugin.RequireDocument = false
    --finaleplugin.LoadLuaOSUtils = true
    return "aaa - luautils text test"
end

require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')
local text = osutils.text

print(osutils._VERSION)

local listing
local file = io.open(finenv.RunningLuaFolderPath().."/".."listing.txt", "r")
if file then
    listing = file:read("*all")
end

local listing_codepage = 850

if listing then
    print(listing)
    local to_utf8 = text.reencode(listing, 850, 10000)
    print(to_utf8)
end
