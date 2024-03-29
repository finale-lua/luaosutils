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

namespace luaosutils
{

static NSString* GetUserShellPath()
{
   uid_t uid = getuid(); // Get the user ID
   struct passwd *pw = getpwuid(uid); // Get the user information
   const char *shell = pw->pw_shell; // Get the shell program path
   return [NSString stringWithUTF8String:shell]; // Convert the path to an NSString
}

bool process_execute(const std::string& cmd, const std::string& dir, std::string& processOutput)
{
   NSTask *task = nil;
   NSString *output = nil;
   @try {
      NSString* nsCmd = [NSString stringWithUTF8String:cmd.c_str()];
      task = [[NSTask alloc] init];
      [task setLaunchPath:GetUserShellPath()];
      [task setArguments:@[@"-c", nsCmd]];
      if (dir.size())
      {
         NSString* dirPath = [NSString stringWithUTF8String:dir.c_str()];
         [task setCurrentDirectoryURL:[NSURL fileURLWithPath:dirPath]];
      }
      
      NSPipe *pipe = [NSPipe pipe];
      [task setStandardOutput:pipe];
      
      NSFileHandle *file = [pipe fileHandleForReading];
      
      NSError* error = nil;
      const bool success = [task launchAndReturnError:&error];
      if (!success)
      {
         NSLog(@"Error returned from launchAndReturnError in process_execute: %@", error);
         return false;
      }
      
      NSData *data = [file readDataToEndOfFile];
      output = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
      
      processOutput = [output UTF8String];
      
      return true;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in process_execute: %@", exception);
   } @finally
   {
#if ! __has_feature(objc_arc)
      [task release];
      [output release];
#endif
   }
   return false;
}

bool process_launch(const std::string& cmd, const std::string& dir)
{
   NSTask *task = nil;
   @try {
      NSString* nsCmd = [NSString stringWithUTF8String:cmd.c_str()];
      task = [[NSTask alloc] init];
      [task setLaunchPath:GetUserShellPath()];
      [task setArguments:@[@"-c", nsCmd]];
      if (dir.size())
      {
         NSString* dirPath = [NSString stringWithUTF8String:dir.c_str()];
         [task setCurrentDirectoryURL:[NSURL fileURLWithPath:dirPath]];
      }
      
      [task setStandardOutput:[NSPipe pipe]];
      [task setStandardError:[NSPipe pipe]];
      NSError* error = nil;
      const bool success = [task launchAndReturnError:&error];
      if (!success)
         NSLog(@"Error returned from launchAndReturnError in process_launch: %@", error);
      return success;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in process_launch: %@", exception);
   } @finally
   {
#if ! __has_feature(objc_arc)
      [task release];
#endif
   }
   return false;
}

void run_event_loop(double timeoutSeconds)
{
   [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:timeoutSeconds]];
}

}
