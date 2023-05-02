//
//  luaosutils_text_os.h
//  luaosutils-static
//
//  Created by Robert Patterson on 2/27/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_text_os_h
#define luaosutils_text_os_h

namespace luaosutils
{

bool text_convert_encoding(const std::string& text, unsigned int fromCodepage, std::string& output, unsigned int toCodepage);
int text_get_default_codepage(std::string& errorMessage);
int text_get_utf8_codepage();

}
#endif /* luaosutils_text_os_h */
