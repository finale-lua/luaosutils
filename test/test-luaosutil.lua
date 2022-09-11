function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils test"
end
    
require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')

for k, v in pairs(osutils) do
    print(tostring(k), tostring(v))
end

function callback(download_successful, urlcontents)
   if download_successful then
   	   local fileout = io.open(finenv.RunningLuaFolderPath().."/gmail.ico", "wb")
       fileout:write(urlcontents)
       fileout:close()
   else
       print(urlcontents)
   end
   finenv.RetainLuaState = false
end

--local session = osutils.download_url("https://ssl.gstatic.com/ui/v1/icons/mail/rfr/gmail.ico", callback)
local session = osutils.download_url("https://mysite.com/test.txt", callback)

finenv.RetainLuaState = true
