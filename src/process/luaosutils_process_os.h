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


namespace luaosutils
{

bool process_execute(const std::string& cmd, const std::string& dir, std::string& processOutput);
bool process_launch(const std::string& cmd, const std::string& dir);
void run_event_loop(double timeoutSeconds);
}
#endif /* luaosutils_process_os_h */
