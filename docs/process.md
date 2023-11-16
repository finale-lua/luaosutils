# The 'process' namespace

The `process` namespace offers functions to launch a separate process. The advantage of these APIs over the standard Lua APIs is that the process is launched *silently*. No console window appears on either macOS or Windows.

The optional folder path for the working directory must be a fully qualified path name. Do not enclose this string in outer quote marks even if the path name contains spaces. If you do, Windows will not recognize it as a path name, and the function will fail. On macOS the functions do not fail, but the outer quote marks are not necessary either. For example, you can pass `finenv.RunningLuaFolderPath()` directly on either operating system. Do not enclose it in quotes, even if the running Lua path contains spaces.

### process.execute\*

Executes a process with the input command line and waits for it to complete. It captures any text the process sends to `stdio` and returns it in a string.

|Input Type|Description|
|----------|-----------|
|string|The command line to execute encoded in UTF-8.|
|(string)|Optional folder path to set as the working directory for the process (also UTF-8).|

|Output Type|Description|
|----------|-----------|
|string|Output from the executed process or `nil` if there was an error.|

On macOS the returned string is encoded in UTF-8.

On Windows, the string is returned unmodified from the output of the process. Lua scripts must handle any character encoding themselves. It may be helpful to use the `&` pipe character to prepend your command with a call to `chcp 65001`. That sets the active code page to UTF-8. As long as the program you run emits text in the active code page, your returned text will be encoded UTF-8.

Example:

```lua
local osutils = require('luaosutils')
local process = osutils.process

if finenv.UI():IsOnWindows() then
    local listing = process.execute('cmd /c chcp 65001 & REG QUERY \"HKLM\\Software\\Microsoft\" /reg:32')
    -- listing is now a string containing a listing of the specified registry key.
end
```

### process.launch\*

Launches a process with the input command line and returns immediately.

|Input Type|Description|
|----------|-----------|
|string|The command line to execute encoded in UTF-8.|
|(string)|Optional folder path to set as the working directory for the process (also UTF-8).|

|Output Type|Description|
|----------|-----------|
|boolean|True if process was successfully launched.|

Note that successfully launching the process does not mean that the command line pointed to a valid program. If your program does not exist, `process.launch` may still return `true`, because it does not wait to see what the result of running the process is.

Example:

```lua
local osutils = require('luaosutils')
local process = osutils.process

if finenv.UI():IsOnMac() then
    -- launch Safari and return immediately
    local success = process.launch("open /Applications/Safari.app")
end
```

### process.list\_dir

Returns a directory listing for the specified directory. 

|Input Type|Description|
|----------|-----------|
|string|The directory to list encoded in UTF-8.|
|(string)|Optional string that contains options for the listing command. These are OS-specific.|

|Output Type|Description|
|----------|-----------|
|string|Output of the list command, encoded in either UTF-8 (macOS) or the default codepage (Windows).|

This function uses `ls` on macOS and `dir` on Windows. This function is not restriced, so unverified code can use it to get a directory listing instead `process.execute` or `io.popen`, both of which are restricted.

Example:

```
local osutils = require('luaosutils')
local process = osutils.process

-- get a file listing of the scripts running path
local dir = process.list_dir(finenv.RunningLuaFolderPath())
```

### process.make\_dir

Makes a new directory if it does not already exist.

|Input Type|Description|
|----------|-----------|
|string|The path of the directory to create encoded in UTF-8.|
|(string)|Optional base directory path in which to create the new directory.|

|Output Type|Description|
|----------|-----------|
|boolean|True if the command was successfully launched.|

You can specify the entire directory path to create in the first paremeter and omit the second. This function uses the `mkdir` command. This function is not restriced, so unverified code can use it to create a directory instead of `process.launch` or `os.execute`, both of which are restricted.

Example:

```
local osutils = require('luaosutils')
local process = osutils.process

-- create a directory called "test" inside the script's folder path
local dir = process.make_dir("test", finenv.RunningLuaFolderPath())
```
