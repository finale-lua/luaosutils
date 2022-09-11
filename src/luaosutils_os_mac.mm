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

OSSESSION_ptr __download_url (const std::string &urlString, __download_callback callback)
{
   NSURL* url = [NSURL URLWithString:[NSString stringWithUTF8String:urlString.c_str()]];
   NSURLSession* session = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]
                                                   delegate:nil
                                                   delegateQueue:[NSOperationQueue mainQueue]]; // mainQueue uses the main app thread
   NSURLSessionDataTask* sessionTask = [session dataTaskWithURL:url
      completionHandler:^(NSData *data, NSURLResponse *response, NSError *error)
      {
         NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;
         NSLog(@"response status code: %ld", (long)[httpResponse statusCode]);
         if (error)
            callback(false, [[error localizedDescription] UTF8String]);
         else
            callback(true, std::string(static_cast<const char *>([data bytes]), [data length]));
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
   return (__bridge void *)(sessionTask);
}

void __cancel_session(OSSESSION_ptr session)
{
   NSURLSessionDataTask* nssession = (__bridge NSURLSessionDataTask*)session;
   if (! [[nssession progress] isFinished])
      [nssession cancel];
}
