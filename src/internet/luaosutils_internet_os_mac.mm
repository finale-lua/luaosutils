//
//  luaosutils_os_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 9/9/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#import <Cocoa/Cocoa.h>

#include "luaosutils.hpp"
#include "internet/luaosutils_internet_os.h"

OSSESSION_ptr https_request(const std::string& requestType, const std::string &urlString, const std::string& postData,
                             const HeadersMap& headers, double timeout, lua_callback callback)
{
   __block bool inProgress = true; // matters only in the synchronous version of this routine
   NSURL* url = [NSURL URLWithString:[NSString stringWithUTF8String:urlString.c_str()]];
   NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
   NSString *method = [NSString stringWithUTF8String:requestType.c_str()];

   // Set the HTTP method based on the user's input
   if ([method isEqualToString:@"get"])
   {
      request.HTTPMethod = @"GET";
   }
   else if ([method isEqualToString:@"post"])
   {
      request.HTTPMethod = @"POST";
      NSData *postDataBytes = [[NSString stringWithUTF8String:postData.c_str()] dataUsingEncoding:NSUTF8StringEncoding];
      [request setHTTPBody:postDataBytes];
   }
   else
   {
      NSLog(@"Invalid method: %@", method);
      assert(false); // offensive programming, since this should never happen
      return nil;
   }
   
   // Set the HTTP headers if any
   for (auto header : headers)
   {
      NSString *key = [NSString stringWithUTF8String:header.first.c_str()];
      NSString *value = [NSString stringWithUTF8String:header.second.c_str()];
      [request addValue:value forHTTPHeaderField:key];
   }

   NSURLSessionDataTask* sessionTask = [[NSURLSession sharedSession] dataTaskWithRequest:request
      completionHandler:^(NSData *data, NSURLResponse *response, NSError *error)
      {
         NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;
         NSLog(@"NSURLSessionDataTask response status code: %ld", (long)[httpResponse statusCode]);
         if (! inProgress) return;
         auto codeBlock = ^{
            if (error)
               callback(false, [[error localizedDescription] UTF8String]);
            else
               callback(true, std::string(static_cast<const char *>([data bytes]), [data length]));
         };
         if (timeout < 0)
            dispatch_async(dispatch_get_main_queue(), codeBlock); // async calls must run on main thread because Lua is not thread-safe
         else
            codeBlock();
         inProgress = false;
      }];
   if (! sessionTask)
   {
      callback(false, [[NSString stringWithFormat:@"Failed to create session for %@.", [url absoluteString]] UTF8String]);
      return nil;
   }
   if ([sessionTask error])
   {
      callback(false, [[NSString stringWithFormat:@"Failed to create session for %@: %@",
                           [url absoluteString], [[sessionTask error] localizedDescription]] UTF8String]);
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

void cancel_session(OSSESSION_ptr session)
{
   NSURLSessionDataTask* nssession = (__bridge NSURLSessionDataTask*)session;
   if (! [[nssession progress] isFinished])
      [nssession cancel];
}

static NSModalResponse InternalRunAlertPanel(NSAlertStyle style, NSString *title, NSString *msgFormat, NSString *defaultButton, NSString *alternateButton, NSString *otherButton)
{
    NSModalResponse retVal = NSModalResponseAbort;
    NSAlert * alert = nil;
    @try
    {
        alert = [[NSAlert alloc] init];
        if (style >= 0) alert.alertStyle = style;
        alert.informativeText = msgFormat;
        alert.messageText = title;
        [alert addButtonWithTitle: defaultButton];
        if (alternateButton) [alert addButtonWithTitle: alternateButton];
        if (otherButton) [alert addButtonWithTitle: otherButton];
        retVal = [alert runModal];
    }
    @catch ( NSException *exc )
    {
       NSLog (@"%@ %@", [exc name], [exc reason]);
    } @finally
    {
#if ! __has_feature(objc_arc)
       [alert release];
#endif
    }
    return retVal;
}

void error_message_box(const std::string &msg)
{
   NSString* messagestring = [NSString stringWithUTF8String:msg.c_str()];
   NSString* titlestring = @"Error";
   InternalRunAlertPanel(NSAlertStyleCritical, titlestring, messagestring, @"OK", nil, nil);
}
