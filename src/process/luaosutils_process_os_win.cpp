//
//  luaosutils_process_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/21/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#include <string>

#include <windows.h>

#include "process/luaosutils_process_os.h"
#include "winutils/luaosutils_winutils.h"

namespace luaosutils
{

/*
static bool getCmdExeFullPath(std::basic_string<WCHAR>& result)
{
   WCHAR sysDir[MAX_PATH];

   // Get the system directory
   if (GetSystemDirectoryW(sysDir, MAX_PATH) == 0) {
      // Handle error
      return false;
   }

   result = sysDir;
   result += L"\\cmd.exe";

   return true;
}
*/

bool process_execute(const std::string& cmd, const std::string& dir, std::string& processOutput)
{
   SECURITY_ATTRIBUTES saAttr;
   HANDLE hRead, hWrite;

   saAttr.nLength = sizeof(saAttr);
   saAttr.bInheritHandle = TRUE;
   saAttr.lpSecurityDescriptor = NULL;

   if (!CreatePipe(&hRead, &hWrite, &saAttr, 0))
   {
      //std::cerr << "Error: Unable to create pipe" << std::endl;
      return false;
   }

   STARTUPINFOW si;
   PROCESS_INFORMATION pi;

   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   si.hStdError = hWrite;
   si.hStdOutput = hWrite;
   si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
   si.wShowWindow = SW_HIDE;

   ZeroMemory(&pi, sizeof(pi));

   std::basic_string<WCHAR> wCmd = utf8_to_WCHAR(cmd.c_str());
   std::basic_string<WCHAR> wDir = utf8_to_WCHAR(dir.c_str());
   const WCHAR* pDir = dir.size() ? wDir.c_str() : NULL;

   // We have to cast away const here because the API doesn't specify const. But it also does not modify the string.
   if (!CreateProcessW(NULL, wCmd.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, pDir, &si, &pi))
   {
#ifdef _DEBUG
      std::string errMessage = get_last_error_as_string();
#endif
      //std::cerr << "Error: Unable to create process" << std::endl;
      CloseHandle(hRead);
      CloseHandle(hWrite);
      return false;
   }

   CloseHandle(hWrite);

   DWORD bytesRead;
   const int bufferSize = 4096;
   char buffer[bufferSize];
   processOutput = "";

   while (true)
   {
      if (!ReadFile(hRead, buffer, bufferSize - 1, &bytesRead, NULL) || bytesRead == 0)
      {
         break;
      }

      buffer[bytesRead] = '\0';
      processOutput += buffer;
   }

   CloseHandle(hRead);

   WaitForSingleObject(pi.hProcess, INFINITE);

   CloseHandle(pi.hThread);
   CloseHandle(pi.hProcess);

   return true;
}

bool process_launch(const std::string& cmd, const std::string& dir)
{
   STARTUPINFOW si;
   PROCESS_INFORMATION pi;

   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   si.dwFlags |= STARTF_USESHOWWINDOW;
   si.wShowWindow = SW_HIDE;

   ZeroMemory(&pi, sizeof(pi));

   std::basic_string<WCHAR> wCmd = utf8_to_WCHAR(cmd.c_str());
   std::basic_string<WCHAR> wDir = utf8_to_WCHAR(dir.c_str());
   const WCHAR* pDir = dir.size() ? wDir.c_str() : NULL;

   // We have to cast away const here because the API doesn't specify const. But it also does not modify the string.
   if (!CreateProcessW(NULL, wCmd.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, pDir, &si, &pi))
   {
#ifdef _DEBUG
      std::string errMessage = get_last_error_as_string();
#endif
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return false;
   }

   // Close process and thread handles to avoid memory leaks
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);
   return true;
}

void run_event_loop(double timeoutSeconds)
{
   ULONGLONG waitTime = std::lround(timeoutSeconds * 1000.0);
   ULONGLONG elapsed = 0;
   MSG msg{};
   DWORD result = 0;
   ULONGLONG start = GetTickCount64();
   while ((result = ::MsgWaitForMultipleObjects(0, NULL, FALSE, static_cast<DWORD>(waitTime - elapsed), QS_ALLINPUT)) != WAIT_FAILED)
   {
       if (result == WAIT_OBJECT_0)
       {
           while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
           {
               ::TranslateMessage(&msg);
               ::DispatchMessage(&msg);
           }
       }
       elapsed = GetTickCount64() - start;
       if (elapsed >= waitTime)
           break;
   }
#ifdef _DEBUG
   std::string errmsg = get_last_error_as_string();
#endif
}

}
