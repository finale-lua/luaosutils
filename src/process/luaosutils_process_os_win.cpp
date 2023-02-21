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

bool __process_execute(const std::string& cmd, std::string& processOutput)
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

   const std::basic_string<WCHAR> wCmd = __utf8_to_WCHAR(cmd.c_str());

   if (!CreateProcessW(NULL, const_cast<LPWSTR>(wCmd.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
   {
      //std::cerr << "Error: Unable to create process" << std::endl;
      CloseHandle(hRead);
      CloseHandle(hWrite);
      return false;
   }

   CloseHandle(hWrite);

   DWORD bytesRead;
   const int bufferSize = 4096;
   char buffer[bufferSize];
   std::string output = "";

   while (true)
   {
      if (!ReadFile(hRead, buffer, bufferSize - 1, &bytesRead, NULL) || bytesRead == 0)
      {
         break;
      }

      buffer[bytesRead] = '\0';
      output += buffer;
   }

   CloseHandle(hRead);

   WaitForSingleObject(pi.hProcess, INFINITE);

   CloseHandle(pi.hThread);
   CloseHandle(pi.hProcess);

   std::basic_string<WCHAR> wOutput = __char_to_WCHAR(output.c_str());
   processOutput = __WCHAR_to_utf8(wOutput.c_str());

   return true;
}

bool __process_launch(const std::string& cmd)
{
   STARTUPINFOW si;
   PROCESS_INFORMATION pi;

   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   si.dwFlags |= STARTF_USESHOWWINDOW;
   si.wShowWindow = SW_HIDE;

   ZeroMemory(&pi, sizeof(pi));

   const std::basic_string<WCHAR> wCmd = __utf8_to_WCHAR(cmd.c_str());

   if (!CreateProcessW(NULL, const_cast<LPWSTR>(wCmd.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
   {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return false;
   }

   // Close process and thread handles to avoid memory leaks
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);
   return true;
}
