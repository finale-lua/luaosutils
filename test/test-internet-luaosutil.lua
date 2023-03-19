function plugindef()
    finaleplugin.RequireDocument = false
    finaleplugin.LoadLuaOSUtils = false
    return "aaa - luautils internet test"
end
    
require('mobdebug').start() -- for ZeroBrane Studio debugging

if finenv.QueryInvokedModifierKeys(finale.CMDMODKEY_ALT + finale.CMDMODKEY_SHIFT) then
    finenv.RetainLuaState = false
    return
end

local osutils = require('luaosutils')
local internet = osutils.internet

local async_call = true

local headers = {
            ["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/94.0.4606.81 Safari/537.36",
            ["Accept-Language"] = "en-US,en;q=0.9"
        }

local url = "https://ssl.gstatic.com/ui/v1/icons/mail/rfr/gmail.ico"
--local url = "https://robertgpatterson.com/-fininfo/-downloads/-usenglish/Fin26Mac.zip"
local filename = (function()
    local str = finale.FCString()
    str.LuaString = url
    local filename = finale.FCString()
    str:SplitToPathAndFile(nil, filename)
    return filename.LuaString
end)()

function callback(download_successful, urlcontents)
   if download_successful then
   	   local fileout = io.open(finenv.RunningLuaFolderPath().."/"..filename, "wb")
       fileout:write(urlcontents)
       fileout:close()
       print("success")
   else
       print(urlcontents)
   end
   finenv.RetainLuaState = false
end

if async_call then
    g_session = internet.get(url, callback, headers)
    finenv.RetainLuaState = true
else
    local success, data = internet.get_sync(url, 5, headers)
    callback(success, data)
end
