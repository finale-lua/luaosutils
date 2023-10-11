//
//  luaosutils_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 9/11/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#include <algorithm>
#include <cassert>

#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>

#include "luaosutils.hpp"
#include "winutils/luaosutils_winutils.h"
#include "internet/luaosutils_internet_os.h"

namespace luaosutils
{


win_request_context::win_request_context(lua_callback callback) :
            callbackFunction(callback), bufferReserve(false),
            hInternet(0), hConnect(0), hRequest(0), hEvent(0),
            readErrorCode(0), timerID(0), statusCode(0), numBytesRead(0)
{
   hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

win_request_context::~win_request_context()
{
   this->SetTimerID(0);
   if (hEvent) CloseHandle(hEvent);
   if (hRequest) InternetCloseHandle(hRequest);
   if (hConnect) InternetCloseHandle(hConnect);
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

bool win_request_context::process_read_complete()
{
   if (!this->numBytesRead)
      return true;
   this->buffer.append(this->readBuf.data(), this->numBytesRead);
   this->readBuf.clear();
   return false;
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

static void ReserveBuffer(win_request_context* session)
{
   if (session->bufferReserve) return;
   session->bufferReserve = -1; // block further attempts to access it.
   CHAR lengthAsText[256];
   DWORD sizeLength = sizeof(lengthAsText);
   if (HttpQueryInfoA(session->hRequest, HTTP_QUERY_CONTENT_LENGTH, lengthAsText, &sizeLength, 0))
   {
      lengthAsText[(std::min)((size_t)sizeLength, sizeof(lengthAsText) - 1)] = 0;
      DWORD length = atoi(lengthAsText);
      if (length)
      {
         session->buffer.reserve(length);
         session->bufferReserve = length;
      }
   }
}

static bool ReadResponseAndTerminate(win_request_context* session)
{
   ReserveBuffer(session);
   while (true)
   {
      assert(session->readBuf.size() == 0);
      session->numBytesRead = 0;
      session->readBuf.resize(4096);
      BOOL result = InternetReadFile(session->hRequest, session->readBuf.data(), static_cast<DWORD>(session->readBuf.size()), &session->numBytesRead);
      if (!result)
      {
         session->readErrorCode = GetLastError();
         return false;
      }
      if (session->process_read_complete())
         break;
   }

   DWORD statusCodeSize = sizeof(session->statusCode);
   BOOL result = HttpQueryInfoA(session->hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &session->statusCode, &statusCodeSize, NULL);
   if (!result)
   {
      session->statusCode = 0;
      session->buffer = GetStringFromLastError(GetLastError(), true);
   }

   InternetCloseHandle(session->hRequest);
   session->hRequest = NULL;
   InternetCloseHandle(session->hConnect);
   session->hConnect = NULL;
   InternetCloseHandle(session->hInternet);
   session->hInternet = NULL;

   return true;
}

static void HandleRequestResult(win_request_context* session, DWORD result, bool errorOnTimeout)
{
   if (errorOnTimeout || result != WAIT_TIMEOUT)
      session->SetTimerID(0); // kill timer if there is one

   switch (result)
   {
      case WAIT_OBJECT_0:
      {
         if (session->readErrorCode)
            session->callbackFunction(false, GetStringFromLastError(session->readErrorCode, true));
         else
            session->callbackFunction(session->statusCode == kHTTPStatusCodeOK, session->buffer);
         break;
      }

      case WAIT_TIMEOUT:
         if (errorOnTimeout)
            session->callbackFunction(false, "Request timed out.");
         break;

      case WAIT_FAILED:
         if (session->readErrorCode)
            session->callbackFunction(false, GetStringFromLastError(session->readErrorCode, true));
         else
            session->callbackFunction(false, GetStringFromLastError(GetLastError(), false));
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
   assert(session && session->hEvent);
   DWORD result = WaitForSingleObject(session->hEvent, 0);
   HandleRequestResult(session, result, false);
   // HandleRequestResult may have destroyed our session, so do not reference it again.
}

void SplitUrl(const std::string& url, std::string& host, std::string& path)
{
   URL_COMPONENTSA urlComponents;
   ZeroMemory(&urlComponents, sizeof(urlComponents));
   urlComponents.dwStructSize = sizeof(urlComponents);
   urlComponents.dwHostNameLength = 1;
   urlComponents.dwUrlPathLength = 1;

   if (InternetCrackUrlA(url.c_str(), static_cast<DWORD>(url.length()), 0, &urlComponents))
   {
      host.assign(urlComponents.lpszHostName, urlComponents.dwHostNameLength);
      path.assign(urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);
   }
   else
   {
      host = "";
      path = "";
   }
}

void CALLBACK WinINetCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
   auto session = reinterpret_cast<win_request_context*>(dwContext);

   switch (dwInternetStatus)
   {
      case INTERNET_STATUS_REQUEST_COMPLETE:
      {
         INTERNET_ASYNC_RESULT* pResult = reinterpret_cast<INTERNET_ASYNC_RESULT*>(lpvStatusInformation);
         if (pResult->dwError == ERROR_SUCCESS)
         {
            if (session->readBuf.size())
               session->process_read_complete();
            // always call ReadResponseAndTerminate one last time, even if the read was complete.
            // this lets the function finish the other side of the read loop.
            if (!ReadResponseAndTerminate(session))
            {
               if (session->readErrorCode == ERROR_IO_PENDING)
               {
                  session->readErrorCode = 0;
                  return;
               }
            }
         }
         else
         {
            // Read error occurred
            session->readErrorCode = pResult->dwError;
         }
         // Signal completion and return
         SetEvent(session->hEvent);
         break;
      }
   }
}

OSSESSION_ptr https_request(const std::string& requestType, const std::string& urlString, const std::string& postData,
                              const HeadersMap& headers, double timeout, lua_callback callback)
{
   OSSESSION_ptr session = OSSESSION_ptr(new win_request_context(callback));

   session->hInternet = InternetOpen(TEXT("Luaosutils WinInet Downloader"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
   if (!session->hInternet)
   {
      callback(false, GetStringFromLastError(GetLastError(), true));
      return nullptr;
   }

   if (InternetSetStatusCallback(session->hInternet, WinINetCallback) == INTERNET_INVALID_STATUS_CALLBACK)
   {
      callback(false, "InternetSetStatusCallback failed with INTERNET_INVALID_STATUS_CALLBACK");
      return nullptr;
   }

   std::string host;
   std::string path;
   SplitUrl(urlString, host, path);

   session->hConnect = InternetConnectA(session->hInternet, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)session.get());
   if (!session->hConnect) {
      callback(false, GetStringFromLastError(GetLastError(), true));
      return nullptr;
   }

   DWORD dwOpenRequestFlags = INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE
      | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_PRAGMA_NOCACHE
      | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_KEEP_CONNECTION;
   session->hRequest = HttpOpenRequestA(session->hConnect, requestType == "get" ? "GET" : "POST", path.c_str(), NULL, NULL, NULL, dwOpenRequestFlags, (DWORD_PTR)session.get());
   if (!session->hRequest)
   {
      callback(false, GetStringFromLastError(GetLastError(), true));
      return nullptr;
   }

   DWORD timeoutValue = 0; // 0 means no timeout or set to a large value
   InternetSetOption(session->hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeoutValue, sizeof(DWORD));
   InternetSetOption(session->hRequest, INTERNET_OPTION_CONNECT_TIMEOUT, &timeoutValue, sizeof(DWORD));

   for (auto& header : headers)
   {
      const std::string value = header.first + ": " + header.second;
      HttpAddRequestHeadersA(session->hRequest, value.c_str(), static_cast<DWORD>(value.size()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
   }

   if (requestType == "post")
   {
      if (headers.find("Content-Type") == headers.end())
      {
         std::string contentType = "application/x-www-form-urlencoded";
         HttpAddRequestHeadersA(session->hRequest, ("Content-Type: " + contentType).c_str(), static_cast<DWORD>(contentType.size()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
      }
      if (headers.find("Content-Length") == headers.end())
      {
         std::string contentLength = std::to_string(postData.size());
         HttpAddRequestHeadersA(session->hRequest, ("Content-Length: " + contentLength).c_str(), static_cast<DWORD>(contentLength.size()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
      }
      session->postData = postData;
      if (!HttpSendRequest(session->hRequest, NULL, 0, session->postData.data(), static_cast<DWORD>(session->postData.size())))
      {
         DWORD err = GetLastError();
         if (err != ERROR_IO_PENDING)
         {
            callback(false, GetStringFromLastError(err, true));
            return nullptr;
         }
      }
   }
   else if (requestType == "get")
   {
      if (!HttpSendRequest(session->hRequest, NULL, 0, NULL, 0))
      {
         DWORD err = GetLastError();
         if (err != ERROR_IO_PENDING)
         {
            callback(false, GetStringFromLastError(err, true));
            return nullptr;
         }
      }
   }
   else
   {
      assert(false); // offensive programming: we should not get here
      return nullptr;
   }

   if (timeout >= 0)
   {
      DWORD result = WaitForSingleObject(session->hEvent, lround(timeout*1000.0));
      HandleRequestResult(session.get(), result, true);
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

void error_message_box(const std::string& msg)
{
   MessageBoxA(__FindTopWindow(), msg.c_str(), "Error", MB_OK);
}

std::string server_name(const std::string& url)
{
   auto urlW = utf8_to_WCHAR(url.c_str());
   WCHAR buffer[1024];
   DWORD bufferSize = DIM(buffer);
   HRESULT result = UrlGetPartW(urlW.c_str(), buffer, &bufferSize, URL_PART_HOSTNAME, 0);
   if (result != S_OK)
      return "";
   return WCHAR_to_utf8(buffer);
}

}
