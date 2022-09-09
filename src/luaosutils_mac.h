//
//  luaosutils_mac.h
//  luaosutils
//
//  Created by Robert Patterson on 9/9/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_mac_h
#define luaosutils_mac_h

#include "luaosutils.hpp"

bool __mac_download_url (const std::string &urlString, __download_callback callback);

#endif /* luaosutils_mac_h */