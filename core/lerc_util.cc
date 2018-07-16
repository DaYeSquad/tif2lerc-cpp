// lerc.cc
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

#include "lerc_util.h"

#include <stdio.h>

#include "tiffio.h"

#include "Lerc.h"

using std::vector;

NS_GAGO_BEGIN

////////////////////////////////////////////////////////////////////////////////
// Lerc, public:

// Tiff --------------------------------------------------------

bool LercUtil::ReadTiffOrDie(const std::string& path_to_file, uint32_t* img_width,
                             uint32_t* img_height, uint32_t* img_dims, DataType* data_type,
                             std::vector<unsigned char>* raw_data) {
  TIFF* tif = TIFFOpen(path_to_file.c_str(), "r");
  if (tif == nullptr) {
    Logger::LogD("ERROR when TIFFOpen %s\n", path_to_file.c_str());
    return false;
  }
  
  uint32_t width = 0;
  uint32_t height = 0;
  tdata_t buf;
  
  uint32_t tiff_dt = 0;
  
  uint32_t bits_per_sample = 0;
  uint32_t dims = 0;
  
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &tiff_dt);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
  TIFFGetField(tif, TIFFTAG_XRESOLUTION, &dims);
  
  Logger::LogD("TIFF width is %d, height is %d, sampleformat is %d, bitsPerSample is %d, dims is %d", width, height, tiff_dt, bits_per_sample, dims);
  Logger::LogD("is TIFF tiled %d", TIFFIsTiled(tif));
  
  if (img_width) *img_width = width;
  if (img_height) *img_height = height;
  if (img_dims) *img_dims = dims;
  
  // Logger::LogD("TIFF sample format is %u, bits per sample is %u", tiff_dt, bits_per_sample);
  
  uint32_t type_size = 0;
  if (tiff_dt == SAMPLEFORMAT_INT) {
    if (bits_per_sample == 8) {
      if (data_type) *data_type = DataType::CHAR;
      type_size = sizeof(int8_t);
    } else if (bits_per_sample == 16) {
      if (data_type) *data_type = DataType::SHORT;
      type_size = sizeof(int16_t);
    } else if (bits_per_sample == 32) {
      if (data_type) *data_type = DataType::INT;
      type_size = sizeof(int32_t);
    } else {
      Logger::LogD("Unknown INT bits per sample %s", path_to_file.c_str());
    }
  } else if (tiff_dt == SAMPLEFORMAT_IEEEFP) {
    if (data_type) *data_type = DataType::FLOAT;
    type_size = sizeof(float);
  } else if (tiff_dt == SAMPLEFORMAT_UINT) {
    if (bits_per_sample == 8) {
      if (data_type) *data_type = DataType::BYTE;
      type_size = sizeof(uint8_t);
    } else if (bits_per_sample == 16) {
      if (data_type) *data_type = DataType::USHORT;
      type_size = sizeof(uint16_t);
    } else if (bits_per_sample == 32) {
      if (data_type) *data_type = DataType::UINT;
      type_size = sizeof(uint32_t);
    } else {
      Logger::LogD("Unknown UINT bits per sample %s", path_to_file.c_str());
    }
  } else {
    Logger::LogD("Unsupported TIFF data format %d, %s", tiff_dt, path_to_file.c_str());
    return false;
  }
  
  // data
  vector<unsigned char>& data = *raw_data;
  
  size_t line_size = TIFFScanlineSize(tif);
  
  for (int row = 0; row < height; ++row) {
    buf = _TIFFmalloc(line_size);
    TIFFReadScanline(tif, buf, row, bits_per_sample);
    
    data.insert(data.end(), static_cast<unsigned char*>(buf), static_cast<unsigned char*>(buf) + line_size);
    
    _TIFFfree(buf);
  }
  
  TIFFClose(tif);
  return true;
}

bool LercUtil::EncodeTiffOrDie(const std::string& path_to_file, const std::string& output_path,
                               double max_z_error, LercVersion lerc_ver, uint16_t band) {
  Logger::LogD("Encoding %s", path_to_file.c_str());
  
  DataType data_type = DataType::UNKNOWN;
  vector<unsigned char> raw_data;
  
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t dims = 0;
  
  if (!ReadTiffOrDie(path_to_file, &width, &height, &dims, &data_type, &raw_data)) {
    return false;
  }
  
  // compress float buffer to lerc
  unsigned int num_bytes_needed = 0;
  unsigned int num_bytes_written = 0;
  LercNS::Lerc lerc;
  
  // convert data type to proper one
  LercNS::Lerc::DataType lerc_dt = static_cast<LercNS::Lerc::DataType>(data_type);
  if (lerc_dt == LercNS::Lerc::DataType::DT_Double ||
      lerc_dt == LercNS::Lerc::DataType::DT_Undefined) {
    Logger::LogD("ERROR input data type %s\n", path_to_file.c_str());
    return false;
  }
  
  if (LercNS::ErrCode::Ok != LercNS::Lerc::ComputeCompressedSize((void*)&raw_data[0],                   // raw image data, row by row, band by band
                              3, lerc_dt, dims,
                              width, height, band,
                              0,                             // set 0 if all pixels are valid
                              max_z_error,                   // max coding error per pixel, or precision
                              num_bytes_needed)) {           // size of outgoing Lerc blob
    Logger::LogD("ERROR when ComputeBufferSize %s\n", path_to_file.c_str());
    return false;
  }
  
  unsigned int num_bytes_blob = num_bytes_needed;
  LercNS::Byte* lerc_buffer = new LercNS::Byte[num_bytes_blob];
  
  Logger::LogD("Try to encode dt: %d w: %d h: %d max_z_error %f band %d", lerc_dt, width, height, max_z_error, band);
  
  if (LercNS::ErrCode::Ok != LercNS::Lerc::Encode((void*)&raw_data[0],            // raw image data, row by row, band by band
                   3, lerc_dt, dims,
                   width, height, band,
                   0,                      // 0 if all pixels are valid
                   max_z_error,            // max coding error per pixel, or precision
                   lerc_buffer,            // buffer to write to, function will fail if buffer too small
                   num_bytes_blob,         // buffer size
                   num_bytes_written)) {   // num bytes written to buffer
    Logger::LogD("ERROR when Encode %s\n", path_to_file.c_str());
    return false;
  }
  
  // write to file
  FILE* file = fopen(output_path.c_str(), "wb");
  fwrite(lerc_buffer, 1, num_bytes_written, file); // write bytes
  fclose(file);
  
  return true;
}

NS_GAGO_END
