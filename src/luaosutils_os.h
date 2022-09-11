//
//  luaosutils_os.h
//  luaosutils
//
//  Created by Robert Patterson on 9/9/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_os_h
#define luaosutils_os_h

#include <memory>
#include <map>

#include "luaosutils.hpp"

#if OPERATING_SYSTEM == MAC_OS
using OSSESSION_ptr = void*;
#endif //OPERATING_SYSTEM == MAC_OS

#if OPERATING_SYSTEM == WINDOWS
#include <wininet.h>
struct win_request_context
{
	__download_callback callbackFunction;
	HINTERNET hInternet;
	HINTERNET hRequest;
	HANDLE hThread;
	bool threadShouldHalt;
	std::string buffer;

	win_request_context(__download_callback callback);
	~win_request_context();

	static win_request_context* get_context_from_timer(UINT_PTR timerID);

	UINT_PTR TimerID() const { return timerID; }
	bool SetTimerID(UINT_PTR id);

private:
	UINT_PTR timerID;

	static std::map< UINT_PTR, win_request_context*>& getTimerMap()
	{
		static std::map< UINT_PTR, win_request_context*> timerMap;
		return timerMap;
	}
};
using OSSESSION_ptr = std::shared_ptr<win_request_context>;
#endif //OPERATING_SYSTEM == WINDOWS

OSSESSION_ptr __download_url (const std::string &urlString, __download_callback callback);
void __cancel_session(OSSESSION_ptr session);

#endif /* luaosutils_os_h */
