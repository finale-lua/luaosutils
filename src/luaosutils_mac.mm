//
//  luaosutils_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 9/9/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#import <Foundation/Foundation.h>

#include "luaosutils.hpp"
#include "luaosutils_mac.h"

bool __mac_download_url (const std::string &urlString, __download_callback callback)
{
   NSURL* url = [NSURL URLWithString:[NSString stringWithUTF8String:urlString.c_str()]];
   NSURLSession* session = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration] delegate:nil delegateQueue:[NSOperationQueue mainQueue]];
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
      return false;
   }
   if ([sessionTask error])
   {
      NSLog(@"Failed to create session for %@: %@", [url absoluteString], [[sessionTask error] localizedDescription]);
      return false;
   }
   [sessionTask resume];
   return true;
}
