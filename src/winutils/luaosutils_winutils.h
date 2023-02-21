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

inline std::basic_string<WCHAR> __utf8_to_WCHAR(const char* inpstr)
{
	std::basic_string<WCHAR> retval;

	const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, inpstr, -1, nullptr, 0) - 1; // remove null-terminator
	if (size > 0)
	{
		retval.resize(size);
		MultiByteToWideChar(CP_UTF8, 0, inpstr, -1, retval.data(), size);
	}
	return retval;
}

inline std::string __WCHAR_to_utf8(const WCHAR* inpstr)
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

#endif //luaosutils_winutils_h
