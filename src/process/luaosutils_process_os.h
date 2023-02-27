//
//  luaosutils_process_os.h
//  luaosutils
//
//  Created by Robert Patterson on 2/21/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_process_os_h
#define luaosutils_process_os_h

bool __process_execute(const std::string& cmd, const std::string& dir, std::string& processOutput);
bool __process_launch(const std::string& cmd, const std::string& dir);

#endif /* luaosutils_process_os_h */
