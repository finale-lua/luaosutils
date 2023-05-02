function plugindef()
    finaleplugin.RequireDocument = false
    finaleplugin.LoadLuaOSUtils = true
    return "aaa - luautils process test"
end

    
require('mobdebug').start() -- for ZeroBrane Studio debugging

print("luaosutils is "..tostring(luaosutils))
local osutils = require('luaosutils')
local process = osutils.process
local text = osutils.text

print(osutils._VERSION)

if finenv.UI():IsOnMac() then
    local result = process.launch("open \"System Information.app\"", "/System/Applications/Utilities/")
    print("launch result", result)
    local listing = process.execute("ls", finenv.RunningLuaFolderPath())
    if listing then
        local fileout = io.open(finenv.RunningLuaFolderPath().."/".."listing.txt", "w")
        fileout:write(listing)
        fileout:close()
    else
        print("execute failed")
    end
end

local function process_execute(str)
    local luaosutils_str = str
    local read_opts = "r"
    if finenv.UI():IsOnWindows() then
        --luaosutils_str = "cmd /c " .. luaosutils_str
        read_opts = "rb"
    end
    local luaosutils_result = process.execute(luaosutils_str)
    local str_1252 = text.convert_encoding(str, text.get_utf8_codepage(), text.get_default_codepage())
    local handle = io.popen(str_1252, read_opts)
    local io_result = handle:read("*all")
    handle:close()
    return luaosutils_result, io_result
end

if finenv.UI():IsOnWindows() then
    --local result = process.launch('cmd /c firefox.exe', "C:/Program Files/Mozilla Firefox/")
    print("launch result", result)
    --os.execute('dir "C:/Program Files"')
    if true then
        -- local listing = process.execute('cmd /c chcp 65001 & /c REG QUERY \"HKLM\\Software\\Microsoft\" /reg:32')
        --local listing = process.execute('cmd /c chcp 65001 & REG QUERY \"HKCU\\SOFTWARE\\RGP\\Finale Plugin Prefs\" /reg:32')
        --local listing, io_result = process_execute('dir "C:\\Program Files\\"')
        local str1 = "ƒüåé"
        local str1_850 = text.convert_encoding(str1, text.get_utf8_codepage(), 850)
        local str1_1252 = text.convert_encoding(str1, text.get_utf8_codepage(), text.get_default_codepage())
        print (str1, str1_850, str1_1252)
        local listing, io_result = process_execute('cmd /c chcp 65001 & echo "ƒüåé"')
        print(listing)
        print(io_result)
        if listing then
            local fileout = io.open(finenv.RunningLuaFolderPath().."/".."listing.txt", "wb")
            fileout:write(listing)
            fileout:close()
        else
            print("execute failed")
        end
    end
end
    