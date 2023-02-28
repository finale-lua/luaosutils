//
//  luaosutils_text_os_win.cpp
//  luaosutils
//
//  Created by Robert Patterson on 2/27/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#include <string>

#include <windows.h>

#include "text/luaosutils_text_os.h"
#include "winutils/luaosutils_winutils.h"

bool __text_reencode(const std::string& text, unsigned int fromCodepage, std::string& output, unsigned int toCodepage)
{
	const int inpSize = MultiByteToWideChar(fromCodepage, MB_ERR_INVALID_CHARS, text.c_str(), -1, nullptr, 0) - 1; // remove null-terminator
	if (inpSize <= 0)
	{
#ifdef _DEBUG
		std::string errMsg = __get_last_error_as_string();
#endif
		return false;
	}
	std::basic_string<WCHAR> wInp;
	wInp.resize(inpSize);
	MultiByteToWideChar(fromCodepage, 0, text.c_str(), -1, wInp.data(), inpSize);
	const int outSize = WideCharToMultiByte(toCodepage, 0, wInp.c_str(), -1, nullptr, 0, NULL, NULL) - 1; // remove null-terminator
	if (outSize <= 0)
	{
#ifdef _DEBUG
		std::string errMsg = __get_last_error_as_string();
#endif
		return false;
	}
	output.resize(outSize);
	WideCharToMultiByte(toCodepage, 0, wInp.c_str(), -1, output.data(), outSize, NULL, NULL);
	return true;
}