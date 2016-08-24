// main.mm
//
// Copyright (c) 2016 Frank Lin (lin.xiaoe.f@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Expect command is : ./LercTilerCmd --input <folder_name> --output <folder_name> --band <band> --maxzerror <max_z_error>

#include <string>

#import <Foundation/Foundation.h>

#include "lerc_util.h"

int main(int argc, const char * argv[]) {
  @autoreleasepool {
    std::string inputFolderPath;
    std::string outputFolderPath;
    uint32_t band = 0;
    double maxZError = 0; // losses
    
    // fake test
    inputFolderPath = "/Users/FrankLin/Downloads/lerc2_test_src/A2016185.ndvi";
    outputFolderPath = "/Users/FrankLin/Downloads/lerc2_test_dist";
    band = 1;
    maxZError = 0.0999;
    
    // parse input arguments
    for (int i = 0; i < argc; ++i) {
      if (0 == strcmp("--input", argv[i])) {
        inputFolderPath = argv[i + 1];
      } else if (0 == strcmp("--output", argv[i])) {
        outputFolderPath = argv[i + 1];
      } else if (0 == strcmp("--band", argv[i])) {
        band = atoi(argv[i + 1]);
      } else if (0 == strcmp("--maxzerror", argv[i])) {
        maxZError = atof(argv[i + 1]);
      }
    }
    
    // give a galance
    NSLog(@"The input folder path is %s, output lerc files will be inside %s, the TIFF band is %d, max Z error given is %f",
          inputFolderPath.c_str(),
          outputFolderPath.c_str(),
          band,
          maxZError);
    
    // create output directory
    BOOL isDir = NO;
    NSString *outputLercFolderPath = [NSString stringWithUTF8String:outputFolderPath.c_str()];
    if (![[NSFileManager defaultManager] fileExistsAtPath:outputLercFolderPath isDirectory:&isDir]) {
      [[NSFileManager defaultManager] createDirectoryAtPath:outputLercFolderPath
                                withIntermediateDirectories:YES
                                                 attributes:nil
                                                      error:nil];
    }
    
    // enumerate all files in directory
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *directoryURL = [NSURL URLWithString:[NSString stringWithUTF8String:inputFolderPath.c_str()]];
    NSArray *keys = [NSArray arrayWithObject:NSURLIsDirectoryKey];
    
    NSDirectoryEnumerator *enumerator = [fileManager
                                         enumeratorAtURL:directoryURL
                                         includingPropertiesForKeys:keys
                                         options:0
                                         errorHandler:^(NSURL *url, NSError *error) {
                                           // Handle the error.
                                           // Return YES if the enumeration should continue after the error.
                                           return YES;
                                         }];
    
    for (NSURL *url in enumerator) {
      NSError *error;
      NSNumber *isDirectory = nil;
      if (! [url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:&error]) {
        // handle error
      } else {
        BOOL isFile = ![isDirectory boolValue];
        if (isFile) {
          NSString *filePath = [url path];
          NSString *fileName = [[filePath lastPathComponent] stringByDeletingPathExtension];
          NSString *fileExt = [[filePath lastPathComponent] pathExtension];
          if (![fileExt isEqualToString:@"tiff"]) {
            NSLog(@"Not a tif file %@", fileName);
            continue;
          }
          
          NSString *outputLercFolder = [filePath stringByReplacingOccurrencesOfString:[NSString stringWithUTF8String:inputFolderPath.c_str()]
                                                                           withString:outputLercFolderPath];
          NSString *outputFilePath = [[outputLercFolder stringByDeletingPathExtension] stringByAppendingPathExtension:@"lerc"];
          NSLog(@"Output file path is %@", outputFilePath);
          
          bool result = gago::LercUtil::EncodeTiffOrDie([filePath UTF8String],
                                                        [outputFilePath UTF8String],
                                                        maxZError,
                                                        gago::LercUtil::LercVersion::V2_3,
                                                        gago::LercUtil::DataType::FLOAT,
                                                        band);
          NSString *successDesc = @"successed";
          if (!result) {
            successDesc = @"failed";
          }
          NSLog(@"%@ %@", fileName, successDesc);
        } else {
          NSString *filePath = [url path];
          NSString *folderPath = [filePath stringByReplacingOccurrencesOfString:[NSString stringWithUTF8String:inputFolderPath.c_str()]
                                                                       withString:outputLercFolderPath];
          NSLog(@"Folder path is %@", folderPath);
          [[NSFileManager defaultManager] createDirectoryAtPath:folderPath
                                    withIntermediateDirectories:YES
                                                     attributes:nil
                                                          error:nil];
        }
      }
    }
    
    NSLog(@"DONE!");
  }
  return 0;
}
