function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils test"
end
    
require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')

local async_call = true

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
   	   local fileout = io.open(finenv.RunningLuaFolderPathAA().."/"..filename, "wb")
       fileout:write(urlcontents)
       fileout:close()
       print("success")
   else
       print(urlcontents)
   end
   finenv.RetainLuaState = false
end

if async_call then
    g_session = osutils.download_url(url, callback)
    finenv.RetainLuaState = true
else
    local success, data = osutils.download_url_sync(url, 5)
    callback(success, data)
end
