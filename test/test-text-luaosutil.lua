function plugindef()
    finaleplugin.RequireDocument = false
    finaleplugin.LoadLuaOSUtils = false
    return "aaa - luautils text test"
end

require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')
local text = osutils.text

print(osutils._VERSION, "Using internal luaosutils: "..tostring(finaleplugin.LoadLuaOSUtils))
print(text.get_utf8_codepage())
print(text.get_default_codepage())

local listing
local file = io.open(finenv.RunningLuaFolderPath().."/".."listing.txt", "rb")
if file then
    listing = file:read("*all")
end

local listing_codepage = 850

if listing then
    --print(listing)
    local to_utf8 = text.convert_encoding(listing, listing_codepage, 1252)
    --print(to_utf8)
end

local str1 = "こんにちは"
local str1_ShiftJS = text.convert_encoding(str1, 65001, 932)
print(str1_ShiftJS) -- prints the string encoded in Shift_JIS
local str1_utf8 = text.convert_encoding(str1_ShiftJS, 932)
print(str1_utf8)
local try_ShiftJS = text.convert_encoding(listing, 932, 932)
if try_ShiftJS then
    print(try_ShiftJS)
else
    print("try_ShiftJS is not valid ShiftJS")
end