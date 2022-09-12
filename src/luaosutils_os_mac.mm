//
//  luaosutils_os_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 9/9/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#import <Foundation/Foundation.h>

#include "luaosutils.hpp"
#include "luaosutils_os.h"

OSSESSION_ptr __download_url (const std::string &urlString, double timeout, __download_callback callback)
{
   __block bool inProgress = true; // matters only in the synchronous version of this routine
   NSURL* url = [NSURL URLWithString:[NSString stringWithUTF8String:urlString.c_str()]];
   NSURLSessionDataTask* sessionTask = [[NSURLSession sharedSession] dataTaskWithURL:url
      completionHandler:^(NSData *data, NSURLResponse *response, NSError *error)
      {
         if (! inProgress) return;
         NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;
         NSLog(@"response status code: %ld", (long)[httpResponse statusCode]);
         auto codeBlock = ^{
            if (error)
               callback(false, [[error localizedDescription] UTF8String]);
            else
               callback(true, std::string(static_cast<const char *>([data bytes]), [data length]));
         };
         if (timeout < 0)
            dispatch_async(dispatch_get_main_queue(), codeBlock);
         else
            codeBlock();
         inProgress = false;
      }];
   if (! sessionTask)
   {
      NSLog(@"Failed to create session for %@", [url absoluteString]);
      return nil;
   }
   if ([sessionTask error])
   {
      NSLog(@"Failed to create session for %@: %@", [url absoluteString], [[sessionTask error] localizedDescription]);
      return nil;
   }
   [sessionTask resume];
   if (timeout >= 0)
   {
      NSTimeInterval startTime = [NSDate timeIntervalSinceReferenceDate];
      while (inProgress)
      {
         [NSThread sleepForTimeInterval:0.05]; // minimum timeout value
         if ([NSDate timeIntervalSinceReferenceDate] >= (startTime + timeout))
         {
            if (! [[sessionTask progress] isFinished])
            {
               inProgress = false;
               [sessionTask cancel];
               callback(false, "Request timed out.");
            }
         }
      }
      sessionTask = nil;
   }
   return (__bridge void *)(sessionTask);
}

void __cancel_session(OSSESSION_ptr session)
{
   NSURLSessionDataTask* nssession = (__bridge NSURLSessionDataTask*)session;
   if (! [[nssession progress] isFinished])
      [nssession cancel];
}
