function plugindef()
    finaleplugin.NoStore = true
    finaleplugin.LoadLuaOSUtils = true
    finaleplugin.RequireDocument = false
    finaleplugin.MinJWLuaVersion = 0.66
    finaleplugin.ExecuteHttpsCalls = true
    return "aaa - OpenAI API Test"
end

require("mobdebug").start()

print("RetainLuaState", finenv.RetainLuaState)

if finenv.QueryInvokedModifierKeys(finale.CMDMODKEY_ALT + finale.CMDMODKEY_SHIFT) then
    finenv.RetainLuaState = false
    return
end

-- note: the actual openai_api_key should be set in the prefix setting for RGP Lua
openai_api_key = openai_api_key or "ABCDEFG"

package.path = finenv.RunningLuaFolderPath() .. "openai-lua/?.lua;" .. package.path

print("cjson global", cjson)

local json = require("cjson")

print("cjson version", json._VERSION)

local async_call = false

local openai = require("openai")

openai.configure(openai_api_key)

local model = "gpt-3.5-turbo" --"davinci"
local prompt = "The quick brown fox jumps"
local temperature = 1.5
local max_tokens = 50

function callback(download_successful, api_result)
   local r = json.decode(api_result)
   if download_successful then
       print(prompt..r.choices[1].text)
   else
       finenv.UI():AlertError(r.error.message, "OpenAI")
   end
   finenv.RetainLuaState = false
end

if async_call then
    g_session = openai.create_completion(model, prompt, temperature, max_tokens, callback)
    if g_session then
        finenv.RetainLuaState = true
    end
else
    local success, response = openai.create_completion(model, prompt, temperature, max_tokens, 5)
    callback(success, response)
    finenv.RetainLuaState = false -- in case it got left open
end

