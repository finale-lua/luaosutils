//
//  luaosutils_text_os_mac.mm
//  luaosutils
//
//  Created by Robert Patterson on 2/28/23.
//  Copyright © 2023 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//
#include <string>

#import <Cocoa/Cocoa.h>

#include "text/luaosutils_text_os.h"

bool __text_convert_encoding(const std::string& text, unsigned int fromCodepage, std::string& output, unsigned int toCodepage)
{
   @try {
      CFStringEncoding cfFromEncoding = CFStringConvertWindowsCodepageToEncoding(fromCodepage);
      if (cfFromEncoding == kCFStringEncodingInvalidId) return false;
      CFStringEncoding cfToEncoding = CFStringConvertWindowsCodepageToEncoding(toCodepage);
      if (cfToEncoding == kCFStringEncodingInvalidId) return false;
      NSStringEncoding nsFromEncoding = CFStringConvertEncodingToNSStringEncoding(cfFromEncoding);
      NSStringEncoding nsToEncoding = CFStringConvertEncodingToNSStringEncoding(cfToEncoding);
      NSData* data = [NSData dataWithBytesNoCopy:(void *)text.data() length:text.size() freeWhenDone:NO];
      NSString* theString = nil;
      BOOL usedLossyConversion = NO;
      [NSString stringEncodingForData:data
                                encodingOptions:@{NSStringEncodingDetectionSuggestedEncodingsKey: @[@(nsFromEncoding)],
                                                  NSStringEncodingDetectionUseOnlySuggestedEncodingsKey: @YES}
                                convertedString:&theString
                            usedLossyConversion:&usedLossyConversion];
      
      /* Decide whether to do anything with `usedLossyConversion` and `determinedEncoding. */
      if (! theString) return false;
      output = [theString cStringUsingEncoding:nsToEncoding];
      return true;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in __text_convert_encoding: %@", exception);
   }
   return false;
}
