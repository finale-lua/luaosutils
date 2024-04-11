# Version History

2.5.0

- added `menu.execute_command_id`

2.4.0

- added `internet.url_escape`
- added `internet.report_errors`
- added `process.run_event_loop`
- tightened up https error reporting

2.3.2

- `internet.launch_website` now internally encloses the URL in quotes if it isn't already quoted. Some complicated URLs (such as Google spreadsheets) require quotes to launch correctly, especially on macOS.

2.3.1

- Added optional custom error handler for when `luaosutils` is embedded. (Not built.)

2.3.0

- Added `crypto` namespace.
- Updated to use Lua 5.4.6.
- Modified Windows HTTP functions to use asynchronous WinINet calls.
- General cleanup of internet utilities.
- Added `internet.close_session` function.

2.2.0

- Added `post` functions to the `internet` namespace, along with the option to specify html headers.
- Rename `download_url` functions as `get`, but maintain `download_url` functions as aliases.
- Windows version of `menu.find_item` now skips '&' on the search string as well as the menu item strings.
- Prebuilt binaries compiled with Lua 5.4
- Added a variant for unverified code that can be loaded with `require('luaosutils.restricted')`.
- Added `launch_website` and `server_name` to the `internet` namespace.
- Added `list_dir` and `make_dir` to the `process` namespace.

2.1.1

- Add explicit Objective-C memory management (macOS) to allow memory-safe embedding in RGP Lua.
- The `window_handle` is now a required parameter for the Windows version of `menu.redraw`.

2.1.0

- Refactored url download functions into `internet` namespace.
- Added `menu` namespace with APIs for rearranging menu options.
- Added `process` namespace with APIs for silent process launching and execution.
- Added `text` namespace with APIs for text encoding.

1.1.1

- Better windows error handling

1.1.0

- original release
