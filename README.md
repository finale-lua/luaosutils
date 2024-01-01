# luaosutils

Luaosutils is an ad-hoc collection of utility functions that access operating system features. It is written in C++ to be used with the Lua language. While the build projects are designed so that it is a simple drop-in to the Lua implementation running on [Finale](https://finalelua.com), it could be used with any Lua implementation if someone wants to take the time to develop the build process.

A primary distinguishing feature of `luaosutils` is that it makes direct API calls on each supported operating system. Other Lua C-libraries tend to avoid OS-specific implementations wherever possible, but that is the whole point with this library. If you examine the `src` directory, you will find parallel implementation files for each operating system. This approach allows for zero external dependencies.

# Installation

The _RGP Lua_ plugin running on Finale embeds a version of `luaosutils`, and no additional installation is necessary to use the embedded version. However, if you wish to use an external version, the following steps apply.

The `release` directory contains two files:

- `luaosutils.dll` (for Windows)
- `luaosutils.dylib` (for macOS)

These are built with Lua 5.4.6.

Choose the appropriate release for your operating system and place it in a folder where Lua can find it with `require`.  For the Finale Lua environment, a simple option is to place `luaosutils.dll` (Windows) or `luaosutils.dylib` (macOS) in the same folder as the script that `requires` it.

If you are bundling `luaosutils` externally with a plugin suite for end users, you may need to build, sign, and notarize it yourself for it to be deployable without error messages on macOS.

# Restricted Mode

\*Items marked with an asterisk are not available in restricted mode. You can load a restricted verision of the library as follows:

```lua
local osutils = require('luaosutils.restricted')
```

This prevents the script from doing things such as changing application menus or executing external code. The restricted mode is primarily useful to environments where `luaosutils` is embedded in the environment. The host app can determine if it trusts the code and either make the full or the restricted version of `luaosutils` available as is appropriate.

# Namespaces

Luaosutils groups related functions into namespaces. You can find more information about each on the following documentation pages.

- [`crypto`](docs/crypto.md) : Functions related to encoding and decoding strings and binary data.
- [`internet`](docs/internet.md) : Functions for accessing resources on the internet with `https` calls.
- [`menu`](docs/menu.md) : Functions for manipulating menu items.
- [`process`](docs/process.md) : Functions for launching other processes.
- [`text`](docs/text.md) : Functions for detecting and modifying text encoding of 8-bit character strings.

# Version History

A list of updates by version is available [here](docs/history.md).

