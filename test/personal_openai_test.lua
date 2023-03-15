function plugindef()
    finaleplugin.NoStore = true
    finaleplugin.LoadLuaOSUtils = false
    finaleplugin.RequireDocument = false
    finaleplugin.MinJWLuaVersion = 0.66
    return "aaa - OpenAI API Test"
end

require("mobdebug").start()

package.path = finenv.RunningLuaFolderPath() .. "openai-lua/?.lua;" .. package.path
package.path = "/Volumes/RGP Data Drive/Robert Folder/Robert/Dev Projects (XCode, Lazarus)/Lua Scripts/src/?.lua;" .. package.path

local openai = require("openai")

openai.configure("ASDASDAS")

local model = "davinci"
local prompt = "The quick brown fox jumps over the lazy dog"
local temperature = 0.5
local max_tokens = 50

local success, response = openai.createCompletion(model, prompt, temperature, max_tokens)
print(success, response)
