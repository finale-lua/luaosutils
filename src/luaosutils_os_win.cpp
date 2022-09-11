//
//  luaosutils_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/11/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//

#include "luaosutils.hpp"
#include "luaosutils_os.h"


#if 0 //OPERATING_SYSTEM == WINDOWS
static DWORD RunWindowsThread(_In_ LPVOID lpParameter)
{
   lua_State * l = reinterpret_cast<lua_State *>(lpParameter);
   //do the download
   return 0;
}
#endif

// On Windows, build the framework in a separate thread, because it needs a bigger stack than WinFin provides.
#if 0 //OPERATING_SYSTEM == WINDOWS
HANDLE thread = CreateThread(nullptr, 0x400000, &RunWindowsThread, l, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);
if (! thread)
   throw std::runtime_error("Unable to create thread to build Lua connection.");
DWORD threadResult = WaitForSingleObject(thread, INFINITE);
if (threadResult != WAIT_OBJECT_0)
   throw std::runtime_error("Thread ended with failure code.");
#endif

OSSESSION_ptr __download_url (const std::string &urlString, __download_callback callback)
{
   
}

void __cancel_session(OSSESSION_ptr session)
{
   
}
