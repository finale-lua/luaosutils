//
//  luaosutils_process_os_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 2/21/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#include <string>
#include <pwd.h>

#import <Cocoa/Cocoa.h>

#include "process/luaosutils_process_os.h"

static NSString* __GetUserShellPath()
{
   uid_t uid = getuid(); // Get the user ID
   struct passwd *pw = getpwuid(uid); // Get the user information
   const char *shell = pw->pw_shell; // Get the shell program path
   return [NSString stringWithUTF8String:shell]; // Convert the path to an NSString
}

bool __process_execute(const std::string& cmd, std::string& processOutput)
{
   @try {
      NSString* nsCmd = [NSString stringWithUTF8String:cmd.c_str()];
      NSTask *task = [[NSTask alloc] init];
      [task setLaunchPath:__GetUserShellPath()];
      [task setArguments:@[@"-c", nsCmd]];
      
      NSPipe *pipe = [NSPipe pipe];
      [task setStandardOutput:pipe];
      
      NSFileHandle *file = [pipe fileHandleForReading];
      
      NSError* error = nil;
      const bool success = [task launchAndReturnError:&error];
      if (!success)
      {
         NSLog(@"Error returned from launchAndReturnError in __process_execute: %@", error);
         return false;
      }

      NSData *data = [file readDataToEndOfFile];
      NSString *output = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
      
      processOutput = [output UTF8String];
      
      return true;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __process_execute: %@", exception);
   }
   return false;
}

bool __process_launch(const std::string& cmd)
{
   @try {
      NSString* nsCmd = [NSString stringWithUTF8String:cmd.c_str()];
      NSTask *task = [[NSTask alloc] init];
      [task setLaunchPath:__GetUserShellPath()];
      [task setArguments:@[@"-c", nsCmd]];
      [task setStandardOutput:[NSPipe pipe]];
      [task setStandardError:[NSPipe pipe]];
      NSError* error = nil;
      const bool success = [task launchAndReturnError:&error];
      if (!success)
         NSLog(@"Error returned from launchAndReturnError in __process_launch: %@", error);
      return success;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __process_launch: %@", exception);
   }
   return false;
}
