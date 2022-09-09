function plugindef()
    finaleplugin.RequireDocument = false
    return "aaa - luautils test"
end
    
require('mobdebug').start() -- for ZeroBrane Studio debugging

local osutils = require('luaosutils')

function callback(download_successful, urlcontents)
   if download_successful then
   	   -- do something with the url contents in result here
   end
end

local download_started = osutils.download_url("https://mysite.com/myfile.txt", "test")

--finenv.RetainLuaState = true
