//
//  luaosutils_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/11/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#include <windows.h>
#include <wininet.h>

#include "luaosutils.hpp"
#include "internet/luaosutils_internet_os.h"

win_request_context::win_request_context(__download_callback callback) :
            callbackFunction(callback),
            hInternet(0), hRequest(0), hThread(0),
            threadShouldHalt(false), timerID(0)
{
}

win_request_context::~win_request_context()
{
   this->SetTimerID(0);
   if (hThread)
   {
      this->threadShouldHalt = true;
      WaitForSingleObject(hThread, INFINITE);
      ::CloseHandle(hThread);
   }
   if (hRequest) InternetCloseHandle(hRequest);
   if (hInternet) InternetCloseHandle(hInternet);
}

win_request_context* win_request_context::get_context_from_timer(UINT_PTR timerID)
{
   auto it = getTimerMap().find(timerID);
   if (it == getTimerMap().end()) return nullptr;
   return it->second;
}

bool win_request_context::SetTimerID(UINT_PTR id)
{
   if (timerID)
   {
      ::KillTimer(NULL, timerID);
      getTimerMap().erase(timerID);
   }
   if (id)
      getTimerMap().emplace(id, this);
   this->timerID = id;
   return (id != 0);
}

inline std::string GetStringFromLastError(DWORD errorCode, bool forWinInet = false)
{
   LPSTR ptr = nullptr;
   HMODULE module = forWinInet ? GetModuleHandle(TEXT("wininet.dll")) : NULL;
   const DWORD reqFlag = module ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM;
   const DWORD numChars = FormatMessageA(reqFlag
                                             | FORMAT_MESSAGE_IGNORE_INSERTS
                                             | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                          module,
                                          errorCode,
                                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                          reinterpret_cast<LPSTR>(&ptr),
                                          0,
                                          NULL);

   std::string retval;
   if (numChars > 0)
   {
      auto deleter = [](void *p) { ::LocalFree(p); };
      std::unique_ptr<CHAR, decltype(deleter)> ptrBuffer(ptr, deleter);
      retval = std::string(ptrBuffer.get(), numChars);
   }
   if (forWinInet && errorCode == ERROR_INTERNET_EXTENDED_ERROR)
   {
      DWORD numExtChars = 0;
      DWORD extErrorCode = 0;
      InternetGetLastResponseInfoA(&extErrorCode, NULL, &numExtChars);
      if (numExtChars)
      {
         numExtChars++; // make room for trailing zero
         std::string extString("", numExtChars);
         InternetGetLastResponseInfoA(&extErrorCode, extString.data(), &numExtChars);
         if (retval.size())
            retval += " ";
         retval += extString;
      }
   }
   if (retval.size())
      return retval;

   return "No error message.";
}

static DWORD RunWindowsThread(_In_ LPVOID lpParameter)
{
   auto session = reinterpret_cast<win_request_context*>(lpParameter);

   CHAR lengthAsText[256];
   DWORD sizeLength = sizeof(lengthAsText);
   if (HttpQueryInfoA(session->hRequest, HTTP_QUERY_CONTENT_LENGTH, lengthAsText, &sizeLength, 0))
   {
      lengthAsText[(std::min)((size_t)sizeLength, sizeof(lengthAsText) - 1)] = 0;
      DWORD length = atoi(lengthAsText);
      if (length)
         session->buffer.reserve(length);
   }

   DWORD numBytesRead = 0;
   char buffer[4096];
   while (InternetReadFile(session->hRequest, buffer, sizeof(buffer), &numBytesRead) && numBytesRead)
   {
      if (session->threadShouldHalt)
         return -1;
      session->buffer.append(buffer, numBytesRead);
   }

   InternetCloseHandle(session->hRequest);
   session->hRequest = NULL;
   InternetCloseHandle(session->hInternet);
   session->hInternet = NULL;

   return 0;
}

static void __HandleThreadResult(win_request_context* session, DWORD result, bool errorOnTimeout)
{
   if (!errorOnTimeout || result != WAIT_TIMEOUT)
      session->SetTimerID(0); // kill timer if there is one

   switch (result)
   {
      case WAIT_OBJECT_0:
      {
         DWORD threadResult = 0;
         BOOL exitResult = GetExitCodeThread(session->hThread, &threadResult);
         if (!exitResult)
            session->callbackFunction(false, GetStringFromLastError(GetLastError()));
         else if (threadResult)
            session->callbackFunction(false, "Download thread failed to download the file.");
         else
            session->callbackFunction(true, session->buffer);
         break;
      }

      case WAIT_TIMEOUT:
         if (errorOnTimeout)
            session->callbackFunction(false, "Request timed out.");
         break;

      default:
         session->callbackFunction(false, GetStringFromLastError(GetLastError()));
         break;
   }

   // if we called the callbackFunction, it destroyed our session, so do not reference it again.
}

static void CALLBACK __TimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
   win_request_context* session = win_request_context::get_context_from_timer(idEvent);
   assert(session && session->hThread);
   DWORD result = WaitForSingleObject(session->hThread, 0);
   __HandleThreadResult(session, result, false);
   // __HandleThreadResult may have destroyed our session, so do not reference it again.
}

OSSESSION_ptr __download_url (const std::string &urlString, double timeout, __download_callback callback)
{
   OSSESSION_ptr session = OSSESSION_ptr(new win_request_context(callback));

   session->hInternet = InternetOpen(TEXT("Luaosutils WinInet Downloader"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
   if (!session->hInternet)
   {
      callback(false, GetStringFromLastError(GetLastError(), true));
      return nullptr;
   }

   session->hRequest = InternetOpenUrlA(session->hInternet, urlString.c_str(), NULL, 0,
                                       INTERNET_FLAG_SECURE |
                                       INTERNET_FLAG_NO_CACHE_WRITE |
                                       INTERNET_FLAG_NO_AUTO_REDIRECT |
                                       INTERNET_FLAG_NO_COOKIES |
                                       INTERNET_FLAG_NO_UI, NULL);
   if (!session->hRequest)
   {
      callback(false, GetStringFromLastError(GetLastError(), true));
      return nullptr;
   }

   session->hThread = CreateThread(NULL, 0, &RunWindowsThread, session.get(), 0, NULL);
   if (!session->hThread)
   {
      callback(false, GetStringFromLastError(GetLastError()));
      return nullptr;
   }

   if (timeout >= 0)
   {
      DWORD result = WaitForSingleObject(session->hThread, lround(timeout*1000.0));
      __HandleThreadResult(session.get(), result, true);
   }
   else
   {
      // Lua is not thread-safe, so we use a timer to check the thread,
      // then call the callback from the main thread (where the timer runs).
      UINT_PTR timerID = ::SetTimer(NULL, 0, USER_TIMER_MINIMUM, &__TimerProc);
      if (!timerID)
         callback(false, GetStringFromLastError(GetLastError()));
      session->SetTimerID(timerID);
      return session->TimerID() ? session : nullptr;
   }

   return nullptr;
}

#include <utility>

HWND __FindTopWindow()
{
   DWORD pid = GetCurrentProcessId();
   std::pair<HWND, DWORD> params = { NULL, pid };

   // Enumerate the windows using a lambda to process each window
   BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
      {
         auto pParams = (std::pair<HWND, DWORD>*)(lParam);

         DWORD processId;
         if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second && GetWindow(hwnd, GW_OWNER) == 0)
         {
            // Stop enumerating
            SetLastError(-1);
            pParams->first = hwnd;
            return FALSE;
         }

         // Continue enumerating
         return TRUE;
      }, (LPARAM)&params);

   if (!bResult && GetLastError() == -1)
      return params.first;

   return NULL;
}

void __error_message_box(const std::string& msg)
{
   MessageBoxA(__FindTopWindow(), msg.c_str(), "Error", MB_OK);
}