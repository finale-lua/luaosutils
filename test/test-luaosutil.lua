function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils test"
end
    
require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')

local complete = false
function callback(download_successful, urlcontents)
   if download_successful then
   	   local fileout = io.open(finenv.RunningLuaFolderPath().."/gmail.ico", "wb")
       fileout:write(urlcontents)
       fileout:close()
   else
       print(urlcontents)
   end
   finenv.RetainLuaState = false
   complete = true
end

local download_started = osutils.download_url("https://ssl.gstatic.com/ui/v1/icons/mail/rfr/gmail.ico", callback)

while not complete do end

--finenv.RetainLuaState = true
