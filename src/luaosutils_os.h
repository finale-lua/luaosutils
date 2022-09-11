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

#include "luaosutils.hpp"

#if OPERATING_SYSTEM == MAC_OS
using OSSESSION_ptr = void*;
#endif //OPERATING_SYSTEM == MAC_OS

#if OPERATING_SYSTEM == WINDOWS
using OSSESSION_ptr = void*;
#endif //OPERATING_SYSTEM == WINDOWS

OSSESSION_ptr __download_url (const std::string &urlString, __download_callback callback);
void __cancel_session(OSSESSION_ptr session);

#endif /* luaosutils_os_h */
