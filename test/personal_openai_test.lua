function plugindef()
    finaleplugin.NoStore = true
    finaleplugin.LoadLuaOSUtils = false
    finaleplugin.RequireDocument = false
    finaleplugin.MinJWLuaVersion = 0.66
    return "aaa - OpenAI API Test"
end

if finenv.QueryInvokedModifierKeys(finale.CMDMODKEY_ALT + finale.CMDMODKEY_SHIFT) then
    finenv.RetainLuaState = false
    return
end

require("mobdebug").start()

-- note: the actual openai_api_key should be set in the prefix setting for RGP Lua
openai_api_key = openai_api_key or "ABCDEFG"

package.path = finenv.RunningLuaFolderPath() .. "openai-lua/?.lua;" .. package.path
package.path = finenv.RunningLuaFolderPath() .. "../../../Lua Scripts/src/?.lua;" .. package.path

local json = require("lunajson.lunajson")

local async_call = true

local openai = require("openai")

openai.configure(openai_api_key)

local model = "davinci"
local prompt = "The quick brown fox jumps"
local temperature = 1.5
local max_tokens = 50

function callback(download_successful, api_result)
   if download_successful then
       local r = json.decode(api_result)
       print(prompt..r.choices[1].text)
   else
       print(api_result)
   end
   finenv.RetainLuaState = false
end

if async_call then
    g_session = openai.create_completion(model, prompt, temperature, max_tokens, callback)
    finenv.RetainLuaState = true
else
    local success, response = openai.create_completion(model, prompt, temperature, max_tokens, 2)
    callback(success, response)
    finenv.RetainLuaState = false -- in case it got left open
end

