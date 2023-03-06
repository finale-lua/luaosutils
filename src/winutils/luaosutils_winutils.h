//
//  luaosutils_winutils.h
//
//  Created by Robert Patterson on 2/21/2023.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
//  This header is only used in Windows

#ifndef luaosutils_winutils_h
#define luaosutils_winutils_h

#include <string>

#include <windows.h>

inline std::basic_string<WCHAR> utf8_to_WCHAR(const char* inpstr)
{
	std::basic_string<WCHAR> retval;

	const int size = MultiByteToWideChar(CP_UTF8, 0, inpstr, -1, nullptr, 0) - 1; // remove null-terminator
	if (size > 0)
	{
		retval.resize(size);
		MultiByteToWideChar(CP_UTF8, 0, inpstr, -1, retval.data(), size);
	}
	return retval;
}

inline std::string WCHAR_to_utf8(const WCHAR* inpstr)
{
	std::string retval;

	int size = WideCharToMultiByte(CP_UTF8, 0, inpstr, -1, NULL, 0, NULL, NULL) - 1; // remove null-terminator
	if (size > 0)
	{
		retval.resize(size);
		WideCharToMultiByte(CP_UTF8, 0, inpstr, -1, retval.data(), size, NULL, NULL);
	}
	return retval;
}

// this function converts and input string to utf8 if necessary
inline std::string __char_to_utf8(const char* inpstr, UINT fallbackCodepage)
{
	int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, inpstr, -1, nullptr, 0) - 1; // remove null-terminator
	if (size > 0) return inpstr;
	size = MultiByteToWideChar(CP_ACP, 0, inpstr, -1, nullptr, 0) - 1; // remove null-terminator
	if (size > 0)
	{
		std::basic_string<WCHAR> wInp;
		wInp.resize(size);
		MultiByteToWideChar(fallbackCodepage, 0, inpstr, -1, wInp.data(), size);
		return WCHAR_to_utf8(wInp.c_str());
	}
	return ""; // error if here
}

inline std::string get_last_error_as_string()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); // no error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

	std::string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}


#endif //luaosutils_winutils_h
