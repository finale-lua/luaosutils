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

namespace luaosutils
{

// WARNING:  get_id_mutex() and get_id_map() must be defined here
//             and not in the header, because when RGPLua includes
//             the header, it is not objective-c. Leaving these
//             functions in the header causes the XCode linker to
//             emit warnings when optimizing for Release.
std::mutex& mac_request_context::get_id_mutex()
{
   static std::mutex idMutex;
   return idMutex;
}

std::map<size_t, mac_request_context*>& mac_request_context::get_id_map()
{
   static std::map<size_t, mac_request_context*> idMap;
   return idMap;
}

OSSESSION_ptr https_request(const std::string& requestType, const std::string &urlString, const std::string& postData,
                            const HeadersMap& headers, double timeout, lua_callback callback)
{
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
   
   OSSESSION_ptr session = OSSESSION_ptr(new mac_request_context(callback));
   size_t sessionId = session->get_id();
   

   NSURLSessionDataTask* sessionTask = [[NSURLSession sharedSession] dataTaskWithRequest:request
               completionHandler:^(NSData *data, NSURLResponse *response, NSError *error)
                  {
                     NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;
                     NSLog(@"NSURLSessionDataTask response status code: %ld", (long)[httpResponse statusCode]);
                     mac_request_context* pSession = luaosutils::mac_request_context::get_context_from_id(sessionId);
                     if (!pSession)
                     {
                        NSLog(@"Completion handler called but session was gone. (User likely canceled it.)");
                        return;
                     }
                     if (error)
                     {
                        NSLog(@"Https request completed with error: %@", [error localizedDescription]);
                        pSession->success = false;
                        pSession->buffer = [[error localizedDescription] UTF8String];
                     }
                     else
                     {
                        pSession->success = [httpResponse statusCode] == kHTTPStatusCodeOK;
                        pSession->buffer = std::string(static_cast<const char *>([data bytes]), [data length]);
                     }
                     auto codeBlock = ^{
                        mac_request_context* pSessionBlock = luaosutils::mac_request_context::get_context_from_id(sessionId);
                        if (!pSessionBlock)
                        {
                           NSLog(@"Async completion handler called but session was gone. (User likely canceled it.)");
                           return;
                        }
                        pSessionBlock->sessionTask = nil; // no need to try to cancel it here, because it's finished.
                        pSessionBlock->complete_request();
                     };
                     if (timeout < 0)
                        dispatch_async(dispatch_get_main_queue(), codeBlock); // async calls must call back on the main thread because Lua is not thread-safe
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
   session->sessionTask = (__bridge void *)(sessionTask);
   [sessionTask resume];
   if (timeout >= 0)
   {
      NSTimeInterval startTime = [NSDate timeIntervalSinceReferenceDate];
      while (! [[sessionTask progress] isFinished])
      {
         if ([NSDate timeIntervalSinceReferenceDate] >= (startTime + timeout))
         {
            if (! [[sessionTask progress] isFinished])
            {
               [sessionTask cancel];
               session->sessionTask = nil;
               session->success = false;
               session->buffer = "Request timed out.";
               break;
            }
         }
         [NSThread sleepForTimeInterval:0.05]; // minimum timeout value
      }
      session->complete_request();
      return nil;
   }
   return session;
}

void cancel_session(void* sessionTask)
{
   NSURLSessionDataTask* nsSession = (__bridge NSURLSessionDataTask*)sessionTask;
   if (nsSession && ! [[nsSession progress] isFinished])
      [nsSession cancel];
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

std::string server_name(const std::string &url)
{
   NSURL* nsUrl = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
   const char * result = [nsUrl.host UTF8String];
   return result ? result : "";
}

std::string url_escape(const std::string &input)
{
   NSString* nsinput = [NSString stringWithUTF8String:input.c_str()];
   NSString *encoded = [nsinput stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]];
   return [encoded UTF8String];
}

} // namespace luaosutils
