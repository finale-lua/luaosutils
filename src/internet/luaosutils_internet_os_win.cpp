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

inline std::string GetStringFromLastError(DWORD errorCode, bool forWinInet = false)
{
   LPWSTR ptr = nullptr;
   HMODULE module = forWinInet && errorCode >= INTERNET_ERROR_BASE && errorCode <= INTERNET_ERROR_LAST
                           ? GetModuleHandle(TEXT("wininet.dll"))
                           : NULL;
   INTERNET_ERROR_BASE;
   const DWORD reqFlag = module ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM;
   const DWORD numChars = FormatMessageW(reqFlag
                                             | FORMAT_MESSAGE_IGNORE_INSERTS
                                             | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                          module,
                                          errorCode,
                                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                          reinterpret_cast<LPWSTR>(&ptr),
                                          0,
                                          NULL);

   std::wstring retval;
   if (numChars > 0)
   {
      auto deleter = [](void *p) { ::LocalFree(p); };
      std::unique_ptr<WCHAR, decltype(deleter)> ptrBuffer(ptr, deleter);
      retval = std::wstring(ptrBuffer.get(), numChars);
   }
   if (forWinInet && errorCode == ERROR_INTERNET_EXTENDED_ERROR)
   {
      DWORD numExtChars = 0;
      DWORD extErrorCode = 0;
      InternetGetLastResponseInfoW(&extErrorCode, NULL, &numExtChars);
      if (numExtChars)
      {
         numExtChars++; // make room for trailing zero
         std::wstring extString(L"", numExtChars);
         InternetGetLastResponseInfoW(&extErrorCode, extString.data(), &numExtChars);
         if (retval.size())
            retval += ' ';
         retval += extString;
      }
   }
   if (retval.size())
      return WCHAR_to_utf8(retval.c_str());

   return "No error message.";
}

static DWORD OnSendRequest(win_request_context* session)
{
   assert(session->state == win_request_state::SEND);
   session->state = win_request_state::ALLOCATE;

   LPVOID postData = (session->postData.size()) ? session->postData.data() : NULL;
   DWORD postDataSize = postData ? static_cast<DWORD>(session->postData.size()) : 0;
   BOOL success = HttpSendRequest(session->hRequest, NULL, 0, postData, postDataSize);
   if (!success) return GetLastError();

   return ERROR_SUCCESS;
}

static DWORD OnAllocateBuffer(win_request_context* session)
{
   assert(session->state == win_request_state::ALLOCATE);
   session->state = win_request_state::READ_CHUNK;

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

   return ERROR_SUCCESS;
}

static DWORD OnReadChunk(win_request_context* session)
{
   assert(session->state == win_request_state::READ_CHUNK);
   session->state = win_request_state::CHUNK_COMPLETE;

   assert(session->readBuf.size() == 0);
   session->readBuf.resize(4096);
   session->numBytesRead = 0;
   BOOL result = InternetReadFile(session->hRequest, session->readBuf.data(), static_cast<DWORD>(session->readBuf.size()), &session->numBytesRead);
   if (!result) return GetLastError();

   return ERROR_SUCCESS;
}

static DWORD OnChunkComplete(win_request_context* session)
{
   assert(session->state == win_request_state::CHUNK_COMPLETE);

   assert(session->readBuf.size() >= session->numBytesRead);
   if (!session->numBytesRead)
      session->state = win_request_state::TERMINATE;
   else
   {
      session->buffer.append(session->readBuf.data(), session->numBytesRead);
      session->state = win_request_state::READ_CHUNK;
   }
   session->readBuf.clear();

   return ERROR_SUCCESS;
}

static DWORD OnTerminate(win_request_context* session)
{
   assert(session->state == win_request_state::TERMINATE);
   session->state = win_request_state::COMPLETE;

   DWORD statusCodeSize = sizeof(session->statusCode);
   BOOL result = HttpQueryInfoA(session->hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &session->statusCode, &statusCodeSize, NULL);
   if (!result)
   {
      session->statusCode = 0;
      session->readErrorCode = GetLastError();
   }

   if (!session->readErrorCode && session->statusCode != HTTP_STATUS_OK)
   {
      DWORD msgSize;
      session->buffer = "";
      // per docs, non-ASCII characters are always encoded CP_ACP, so we'll need to bounce it to utf-8
      if (!HttpQueryInfoA(session->hRequest, HTTP_QUERY_STATUS_TEXT, NULL, &msgSize, NULL) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
         std::string msg(msgSize, '\0');
         if (HttpQueryInfoA(session->hRequest, HTTP_QUERY_STATUS_TEXT, msg.data(), &msgSize, NULL))
            session->buffer = __char_to_utf8(msg.c_str());
      }
      else
         session->buffer = "Request returned status " + std::to_string(session->statusCode) + ".";
   }

   InternetCloseHandle(session->hRequest);
   session->hRequest = NULL;
   InternetCloseHandle(session->hConnect);
   session->hConnect = NULL;
   InternetCloseHandle(session->hInternet);
   session->hInternet = NULL;

   return ERROR_SUCCESS;
}

static void ProcessRequest(win_request_context* session, DWORD errorCode)
{
   while (errorCode == ERROR_SUCCESS && session->state != win_request_state::COMPLETE)
   {
      switch (session->state)
      {
         case win_request_state::SEND:
            errorCode = OnSendRequest(session);
            break;
 
         case win_request_state::ALLOCATE:
            errorCode = OnAllocateBuffer(session);
            break;

         case win_request_state::READ_CHUNK:
            errorCode = OnReadChunk(session);
            break;

         case win_request_state::CHUNK_COMPLETE:
            errorCode = OnChunkComplete(session);
            break;

         case win_request_state::TERMINATE:
            errorCode = OnTerminate(session);
            break;

         case win_request_state::COMPLETE:
            break;
      }
   }

   if (errorCode == ERROR_IO_PENDING)
      return;

   // if we get here, everything is finished.

   if (errorCode != ERROR_SUCCESS)
      session->readErrorCode = errorCode;

   SetEvent(session->hEvent);
}

static void HandleRequestResult(win_request_context* session, DWORD result, bool errorOnTimeout)
{
   if (errorOnTimeout || result != WAIT_TIMEOUT)
      session->set_timer_id(0); // kill timer if there is one

   switch (result)
   {
      case WAIT_OBJECT_0:
      {
         if (session->readErrorCode)
            session->callbackFunction(false, GetStringFromLastError(session->readErrorCode, true));
         else
            session->callbackFunction(session->statusCode == HTTP_STATUS_OK, session->buffer);
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

   // if we called the callbackFunction, it may have destroyed our session, so do not reference it again.
}

static void CALLBACK __TimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
   win_request_context* session = win_request_context::get_context_from_timer(idEvent);
   assert(session->hEvent);
   DWORD result = WaitForSingleObject(session->hEvent, 0);
   HandleRequestResult(session, result, false);
   // HandleRequestResult may have destroyed our session, so do not reference it again.
}

void SplitUrl(const std::string& url, std::string& host, std::string& path)
{
   URL_COMPONENTSA urlComponents{};
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
         ProcessRequest(session, pResult->dwError);
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
   }

   ProcessRequest(session.get(), ERROR_SUCCESS);

   if (timeout >= 0)
   {
      DWORD result = WaitForSingleObject(session->hEvent, std::lround(timeout*1000.0));
      HandleRequestResult(session.get(), result, true);
   }
   else
   {
      // Lua is not thread-safe, so we use a timer to check the status of the request,
      // then call the callback from the main thread (where the timer runs).
      UINT_PTR timerID = ::SetTimer(NULL, 0, USER_TIMER_MINIMUM, &__TimerProc);
      if (!timerID)
         callback(false, GetStringFromLastError(GetLastError()));
      session->set_timer_id(timerID);
      assert(session->get_timer_id());
      return session;
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

std::string url_escape(const std::string& input)
{
   auto inputW = utf8_to_WCHAR(input.c_str());
   std::wstring result(256, '\0');
   DWORD size = static_cast<DWORD>(result.size());
   constexpr DWORD flags = URL_ESCAPE_PERCENT | URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_AS_UTF8;
   HRESULT hr = UrlEscapeW(inputW.c_str(), result.data(), &size, flags);
   if (hr == E_POINTER)
   {
      result.resize(size);
      hr = UrlEscapeW(inputW.c_str(), result.data(), &size, flags);
   }
   if (!SUCCEEDED(hr)) return "";

   return WCHAR_to_utf8(result.c_str());
}

}
