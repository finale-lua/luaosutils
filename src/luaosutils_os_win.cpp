//
//  luaosutils_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/11/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//

#include <windows.h>
#include <wininet.h>

#include "luaosutils.hpp"
#include "luaosutils_os.h"

win_request_context::win_request_context() : hInternet(0), hRequest(0)
{
   memset(&ib, sizeof(ib), 0);
   ib.dwStructSize = sizeof(ib);
}

win_request_context::~win_request_context()
{
   if (ib.lpvBuffer) delete [] ib.lpvBuffer;
   if (hRequest) InternetCloseHandle(hRequest);
   if (hInternet) InternetCloseHandle(hInternet);
}

void win_request_context::SetRequest(HINTERNET request)
{
   if (request)
   {
      assert(!this->hRequest);
      DWORD length = 0;
      DWORD sizeLength = sizeof(length);
      if (!HttpQueryInfo(request, HTTP_QUERY_CONTENT_LENGTH, &length, &sizeLength, 0))
         length = 16 * 1024; // if we can't get the length, requisition 16K for downloading.;
      ib.lpvBuffer = new char[length];
      ib.dwBufferLength = length;
      this->hRequest = request;  
   }
}

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

static VOID CALLBACK __CallBack(
   __in HINTERNET hInternet,
   __in DWORD_PTR dwContext,
   __in DWORD dwInternetStatus,
   __in_bcount(dwStatusInformationLength) LPVOID lpvStatusInformation,
   __in DWORD dwStatusInformationLength
)
{

}

OSSESSION_ptr __download_url (const std::string &urlString, __download_callback callback)
{
   OSSESSION_ptr session = OSSESSION_ptr(new win_request_context);
   session->hInternet = InternetOpen(TEXT("Luaosutils WinInet Downloader"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
   if (!session->hInternet) return nullptr;

   INTERNET_STATUS_CALLBACK sessionCallback = InternetSetStatusCallback(session->hInternet, (INTERNET_STATUS_CALLBACK)__CallBack);

   if (sessionCallback == INTERNET_INVALID_STATUS_CALLBACK)
      return nullptr;

   session->SetRequest(InternetOpenUrlA(session->hInternet, urlString.c_str(), NULL, 0,
                                       INTERNET_FLAG_SECURE |
                                       INTERNET_FLAG_NO_CACHE_WRITE |
                                       INTERNET_FLAG_NO_AUTO_REDIRECT |
                                       INTERNET_FLAG_NO_COOKIES |
                                       INTERNET_FLAG_NO_UI, (DWORD_PTR)session.get()));

   if (!session->hRequest)
   {
      if (GetLastError() != ERROR_IO_PENDING)
         return nullptr;
   }

   return session;
}

void __cancel_session(OSSESSION_ptr session)
{
   
}
