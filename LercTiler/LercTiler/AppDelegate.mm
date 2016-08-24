//
//  AppDelegate.m
//  LercTiler
//
//  Created by Frank Lin on 8/10/16.
//  Copyright © 2016 Frank Lin. All rights reserved.
//

#import "AppDelegate.h"

#include "Lerc/Lerc.h"
#include "png.h"
#include "tiffio.h"
#include "lerc_util.h"

NSString *inputTiff2Path = @"/Users/FrankLin/Documents/Xcode/gago/geotiff2lerc/test.tif";
NSString *inputTiffPath = @"/Users/FrankLin/Downloads/lerc2_test_src/A2016185.ndvi/3/6/3.tiff";

NSString *inputPngPath = @"/Users/FrankLin/Documents/Xcode/gago/png2lerc/test2.png";

NSString *outputLercPath = @"/Users/FrankLin/Documents/Xcode/gago/geotiff2lerc/test.lerc";
//NSString *outputLercPath = @"/Users/FrankLin/Downloads/lerc2_test_dist/4/13/5.lerc";
//NSString *outputLercPath = @"/Users/FrankLin/Downloads/lerc2_test_dist/3/6/3.lerc";
//NSString *outputLercPath = @"/Users/FrankLin/Downloads/lerc2_test_dist/3/7/3.lerc";

NSString *jsonOutputPath = @"/Users/FrankLin/Documents/Xcode/gago/geotiff2lerc/test.json";

typedef struct
{
  const unsigned char * data;
  ssize_t size;
  int offset;
}tImageSource;

static void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
  tImageSource* isource = (tImageSource*)png_get_io_ptr(png_ptr);
  
  if((int)(isource->offset + length) <= isource->size)
  {
    memcpy(data, isource->data+isource->offset, length);
    isource->offset += length;
  }
  else
  {
    png_error(png_ptr, "pngReaderCallback failed");
  }
}

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
   [self testLercUtils];
  [self testReadLerc2Info];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
  // Insert code here to tear down your application
}

- (void)testLercUtils {
  bool result = gago::LercUtil::EncodeTiffOrDie([inputTiffPath UTF8String],
                                                [outputLercPath UTF8String],
                                                0,
                                                gago::LercUtil::LercVersion::V2_3,
                                                gago::LercUtil::DataType::UNKNOWN,
                                                1);
  NSLog(@"Tiff to lerc successed %d", result);
}

- (void)testTiffToLerc {
  TIFF* tif = TIFFOpen([inputTiffPath UTF8String], "r");
  if (tif == nullptr) {
    printf("ERROR when TIFFOpen\n");
    return;
  }
  
  uint32_t width = 0;
  uint32_t height = 0;
  tdata_t buf;
  int color_space = 0;
  
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif, EXIFTAG_COLORSPACE, &color_space);

  // data
  int* data = new int[width * height];

  size_t line_size = TIFFScanlineSize(tif);
  printf("scan line size is %zu\n", line_size);
  
  for (int row = 0; row < height; ++row) {
    buf = _TIFFmalloc(line_size);
    TIFFReadScanline(tif, buf, row);
    
    memcpy(data + width * row, buf, line_size);
    
    _TIFFfree(buf);
  }
  
  TIFFClose(tif);
  
  // compress float buffer to lerc
  double max_z_error_wanted = 0.1;
  double eps = 0.0001;    // safety margin (optional), to account for finite floating point accuracy
  double max_z_error = max_z_error_wanted - eps;
  
  size_t num_bytes_needed = 0;
  size_t num_bytes_written = 0;
  LercNS::Lerc lerc;
  
  int band = 1; // grayscale
  
  if (!lerc.ComputeBufferSize((void*)data,    // raw image data, row by row, band by band
                              LercNS::Lerc::DT_Float,
                              width, height, band,
                              0,                         // set 0 if all pixels are valid
                              max_z_error,                 // max coding error per pixel, or precision
                              num_bytes_needed)) {           // size of outgoing Lerc blob
    printf("ComputeBufferSize failed\n");
  }
  
  size_t num_bytes_blob = num_bytes_needed;
  Byte* lerc_buffer = new Byte[num_bytes_blob];
  
  if (!lerc.Encode((void*)data,    // raw image data, row by row, band by band
                   LercNS::Lerc::DT_Float,
                   width, height, band,
                   0,                   // 0 if all pixels are valid
                   max_z_error,           // max coding error per pixel, or precision
                   lerc_buffer,           // buffer to write to, function will fail if buffer too small
                   num_bytes_blob,        // buffer size
                   num_bytes_written)) {   // num bytes written to buffer
    printf("Encode failed\n");
  }
  
  // write to file
  NSData *outputLercData = [NSData dataWithBytes:lerc_buffer length:num_bytes_written];
  [outputLercData writeToFile:outputLercPath atomically:NO];
  
  // clean memory
  delete[] data;
}

- (void)testPngToLerc {
  // input png file
  NSData *inputPngFile = [NSData dataWithContentsOfFile:inputPngPath
                                                options:0
                                                  error:nil];
  unsigned char* input_png_data = (unsigned char*)[inputPngFile bytes];
  size_t input_png_size = (size_t)[inputPngFile length];
  NSLog(@"Input png file size is %zu", input_png_size);
  
  // read raw data of png file
  png_structp png_ptr = nullptr;
  png_infop png_info_ptr = nullptr;
  
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (!png_ptr) printf("ERROR when png_create_read_struct\n");
  
  png_info_ptr = png_create_info_struct(png_ptr);
  if (!png_info_ptr) printf("ERROR when png_create_info_struct\n");
  
  // set png callback function ? (opt?)
  tImageSource image_source;
  image_source.data = input_png_data;
  image_source.size = input_png_size;
  image_source.offset = 0;
  png_set_read_fn(png_ptr, &image_source, pngReadCallback);
  
  // read png file info
  png_read_info(png_ptr, png_info_ptr);
  int width = png_get_image_width(png_ptr, png_info_ptr);
  int height = png_get_image_height(png_ptr, png_info_ptr);
  png_byte bit_depth = png_get_bit_depth(png_ptr, png_info_ptr);
  png_uint_32 color_type = png_get_color_type(png_ptr, png_info_ptr);
  
  printf("png image width is %d, height is %d , color type is %d, bit depth is %d \n", width, height, color_type, bit_depth);
  
  // read png image data
  png_size_t row_bytes;
  png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  row_bytes = png_get_rowbytes(png_ptr, png_info_ptr);
  
  size_t data_len = row_bytes * height;
  unsigned char* data = static_cast<unsigned char*>(malloc(data_len * sizeof(unsigned char)));
  if (!data) {
    if (row_pointers != nullptr) {
      free(row_pointers);
    }
  }
  
  for (uint16_t i = 0; i < height; ++i) {
    row_pointers[i] = data + i * row_bytes;
  }
  png_read_image(png_ptr, row_pointers);
  png_read_end(png_ptr, nullptr);
  
  if (row_pointers != nullptr) {
    free(row_pointers);
  }
  if (png_ptr) {
    png_destroy_read_struct(&png_ptr, png_info_ptr ? &png_info_ptr : 0, 0);
  }
  
  // converts png to lerc
  size_t num_bytes_needed = 0;
  size_t num_bytes_written = 0;
  LercNS::Lerc lerc;
  
  // get band
  int band = 0;
  if (color_type == PNG_COLOR_TYPE_RGB) {
    band = 3;
  } else if (color_type == PNG_COLOR_TYPE_GRAY) {
    band = 1;
  } else if (color_type == PNG_COLOR_TYPE_RGBA) {
    band = 4;
  }
  printf("png band is %d\n", band);
  
  LercNS::BitMask bit_mask(width, height);
  bit_mask.SetAllValid();
  
  double max_z_error_wanted = 0.1;
  double eps = 0.0001;    // safety margin (optional), to account for finite floating point accuracy
  double max_z_error = max_z_error_wanted - eps;
  
  if (!lerc.ComputeBufferSize(data,
                              LercNS::Lerc::DT_Byte,
                              width, height, band,
                              0, // 0 if all pixels are valid
                              max_z_error,
                              num_bytes_needed)) {
    printf("lerc ComputeBufferSize failed \n");
    return;
  } else {
    printf("lerc ComputeBufferSize bytes needed : %zu \n", num_bytes_needed);
  }
  
  size_t num_bytes_blob = num_bytes_needed;
  Byte* lerc_output_buffer = new Byte[num_bytes_blob];
  
  if (!lerc.Encode(data,
                   LercNS::Lerc::DT_Byte,
                   width, height, band,
                   0, // 0 if all pixels are valid
                   max_z_error,
                   lerc_output_buffer,
                   num_bytes_blob,
                   num_bytes_written)) {
    printf("lerc Encode failed \n");
    return;
  } else {
    printf("lerc Encode write bytes : %zu \n", num_bytes_written);
  }
  
  // write to file
  NSData *outputLercData = [NSData dataWithBytes:lerc_output_buffer length:num_bytes_written];
  [outputLercData writeToFile:outputLercPath atomically:NO];
}

- (void)testEsriSampleFloatToLerc {
  int h = 512;
  int w = 512;
  
  float* zImg = new float[w * h];
  memset(zImg, 0, w * h * sizeof(float));
  
  LercNS::BitMask bitMask(w, h);
  bitMask.SetAllValid();
  
  for (int k = 0, i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++, k++)
    {
      zImg[k] = sqrt((float)(i * i + j * j));    // smooth surface
      zImg[k] += rand() % 20;    // add some small amplitude noise
      
      if (j % 100 == 0 || i % 100 == 0)    // set some void points
        bitMask.SetInvalid(k);
    }
  }
  
  
  // compress into byte arr
  
  double maxZErrorWanted = 0.1;
  double eps = 0.0001;    // safety margin (optional), to account for finite floating point accuracy
  double maxZError = maxZErrorWanted - eps;
  
  size_t numBytesNeeded = 0;
  size_t numBytesWritten = 0;
  LercNS::Lerc lerc;
  
  if (!lerc.ComputeBufferSize((void*)zImg,    // raw image data, row by row, band by band
                              LercNS::Lerc::DT_Float,
                              w, h, 1,
                              &bitMask,                  // set 0 if all pixels are valid
                              maxZError,                 // max coding error per pixel, or precision
                              numBytesNeeded))           // size of outgoing Lerc blob
  {
    printf("ComputeBufferSize failed\n");
  }
  
  size_t numBytesBlob = numBytesNeeded;
  Byte* pLercBlob = new Byte[numBytesBlob];
  
  if (!lerc.Encode((void*)zImg,    // raw image data, row by row, band by band
                   LercNS::Lerc::DT_Float,
                   w, h, 1,
                   &bitMask,           // 0 if all pixels are valid
                   maxZError,           // max coding error per pixel, or precision
                   pLercBlob,           // buffer to write to, function will fail if buffer too small
                   numBytesBlob,        // buffer size
                   numBytesWritten))    // num bytes written to buffer
  {
    printf("Encode failed\n");
  }
}

- (void)testEsriSampleByteToLerc {
  int h = 713;
  int w = 257;
  LercNS::Lerc lerc;
  size_t numBytesNeeded = 0;
  size_t numBytesWritten = 0;
  
  Byte* byteImg = new Byte[w * h * 3];
  memset(byteImg, 0, w * h * 3);
  
  size_t numBytesBlob = numBytesNeeded;
  Byte* pLercBlob = new Byte[numBytesBlob];
  
  for (int iBand = 0; iBand < 3; iBand++)
  {
    Byte* arr = byteImg + iBand * w * h;
    for (int k = 0, i = 0; i < h; i++)
      for (int j = 0; j < w; j++, k++)
        arr[k] = rand() % 30;
  }
  
  // encode
  
  if (!lerc.ComputeBufferSize((void*)byteImg, LercNS::Lerc::DT_Byte, w, h, 3, 0, 0, numBytesNeeded))
    printf("ComputeBufferSize failed\n");
  
  numBytesBlob = numBytesNeeded;
  pLercBlob = new Byte[numBytesBlob];
  
  if (!lerc.Encode((void*)byteImg,    // raw image data, row by row, band by band
                   LercNS::Lerc::DT_Byte,
                   w, h, 3,
                   0,                   // 0 if all pixels are valid
                   0,                   // max coding error per pixel, or precision
                   pLercBlob,           // buffer to write to, function will fail if buffer too small
                   numBytesBlob,        // buffer size
                   numBytesWritten))    // num bytes written to buffer
  {
    printf("Encode failed\n");
  }
}

- (void)testReadLerc2Info {
  NSData *lerc2Data = [NSData dataWithContentsOfFile:outputLercPath];
  
  struct LercNS::Lerc::LercInfo lerc_info;
  
  LercNS::Lerc lerc;
  bool success = lerc.GetLercInfo(static_cast<const LercNS::Byte*>([lerc2Data bytes]),
                                  [lerc2Data length],
                                  lerc_info);
  if (!success) {
    NSLog(@"Read LERC file failed");
    return;
  }
  
  // LercNS::Byte *fakedBytes = static_cast<LercNS::Byte*>(malloc(lerc_info.nCols * lerc_info.nRows * lerc_info.nBands));
  void *uncompressedData = malloc(lerc_info.nCols * lerc_info.nRows * lerc_info.nBands * sizeof(float));
  memset(uncompressedData, 0, lerc_info.nCols * lerc_info.nRows * lerc_info.nBands * sizeof(float));
  const void *inputLerc2DataBytes = [lerc2Data bytes];
  lerc.Decode(static_cast<const LercNS::Byte*>(inputLerc2DataBytes),
              [lerc2Data length],
              0,
              lerc_info.nCols,
              lerc_info.nRows,
              lerc_info.nBands,
              lerc_info.dt,
              uncompressedData);
  
  NSLog(@"The lerc version is %d, num valid pixels is %d", lerc_info.version, lerc_info.numValidPixel);
  
  NSMutableArray *valuesArray = [NSMutableArray array];
  int numNotZero = 0;
  int* values = static_cast<int*>(uncompressedData);
  for (int i = 0; i < 1001 * 1001; i++) {
    [valuesArray addObject:@(values[i])];
    if (values[i] != 0) {
      //printf("index %d is %d\n", i, values[i]);
      numNotZero++;
    }
  }
  printf("num not zero is %d", numNotZero);
  
  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
  [dict setValue:valuesArray forKey:@"values"];
  NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:0 error:nil];
  [jsonData writeToFile:jsonOutputPath atomically:NO];
  
  free(uncompressedData);
}

@end
