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

bool text_convert_encoding(const std::string& text, unsigned int fromCodepage, std::string& output, unsigned int toCodepage)
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
      [[maybe_unused]]BOOL determinedEncoding = [NSString stringEncodingForData:data
                                encodingOptions:@{NSStringEncodingDetectionSuggestedEncodingsKey: @[@(nsFromEncoding)],
                                                  NSStringEncodingDetectionUseOnlySuggestedEncodingsKey: @YES}
                                convertedString:&theString
                            usedLossyConversion:&usedLossyConversion];
      
      /* Decide whether to do anything with `usedLossyConversion` and `determinedEncoding`. */
      if (! theString) return false;
      const char* pOutput = [theString cStringUsingEncoding:nsToEncoding];
      if (!pOutput) return false;
      output = pOutput;
      return true;
   } @catch (NSException *exception)
   {
      NSLog(@"Caught exception in text_convert_encoding: %@", exception);
   }
   return false;
}

int text_get_utf8_codepage()
{
   return CFStringConvertEncodingToWindowsCodepage(kCFStringEncodingUTF8);
}

int text_get_default_codepage(std::string& errorMessage)
{
   errorMessage = "";
   return text_get_utf8_codepage();
}
