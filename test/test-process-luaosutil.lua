function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils process test"
end

    
require('mobdebug').start() -- for ZeroBrane Studio debugging

print("luaosutils is "..tostring(luaosutils))
local osutils = require('luaosutils')
local process = osutils.process

print(osutils._VERSION)

if finenv.UI():IsOnMac() then
    --local result = process.launch("open /Applications/Safari.app")
    --print("launch result", result)
    local listing = process.execute("ls -l /Applications")
    if listing then
        local fileout = io.open(finenv.RunningLuaFolderPath().."/".."listing.txt", "w")
        fileout:write(listing)
        fileout:close()
    else
        print("execute failed")
    end
end

if finenv.UI():IsOnWindows() then
    local result = process.launch("\"C:/Program Files/Mozilla Firefox/firefox.exe\"")
    print("launch result", result)
    --os.execute('dir "C:/Program Files"')
    if true then
        local listing = process.execute('cmd /c dir "C:/Program Files"')
        if listing then
            local fileout = io.open(finenv.RunningLuaFolderPath().."/".."listing.txt", "w")
            fileout:write(listing)
            fileout:close()
        else
            print("execute failed")
        end
    end
end
    