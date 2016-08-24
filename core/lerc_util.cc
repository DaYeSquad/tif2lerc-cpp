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

#include "tiffio.h"

#include "Lerc/Lerc.h"

NS_GAGO_BEGIN

////////////////////////////////////////////////////////////////////////////////
// Lerc, public:

// Tiff --------------------------------------------------------

bool LercUtil::EncodeTiffOrDie(const std::string& path_to_file, const std::string& output_path,
                               double max_z_error, LercVersion lerc_ver, DataType data_type,
                               uint16_t band) {
  Logger::LogD("Encoding %s", path_to_file.c_str());
  TIFF* tif = TIFFOpen(path_to_file.c_str(), "r");
  if (tif == nullptr) {
    Logger::LogD("ERROR when TIFFOpen %s\n", path_to_file.c_str());
    return false;
  }
  
  uint32_t width = 0;
  uint32_t height = 0;
  tdata_t buf;
  
  uint32_t tiff_dt = 0;
  
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif, TIFFTAG_DATATYPE, &tiff_dt);
  
  uint32_t type_size = 0;
  if (tiff_dt == SAMPLEFORMAT_INT) {
    data_type = DataType::INT;
    type_size = 4;
  } else if (tiff_dt == SAMPLEFORMAT_IEEEFP) {
    data_type = DataType::FLOAT;
    type_size = 4;
  } else {
    Logger::LogD("Unsupported TIFF data format");
    return false;
  }
  
  // data
  unsigned char* data = new unsigned char[width * height * type_size];
  
  size_t line_size = TIFFScanlineSize(tif);
  
  for (int row = 0; row < height; ++row) {
    buf = _TIFFmalloc(line_size);
    TIFFReadScanline(tif, buf, row);
    
    memcpy(data + width * row * type_size, buf, line_size);
    
    _TIFFfree(buf);
  }
  
  TIFFClose(tif);
  
  // compress float buffer to lerc
  size_t num_bytes_needed = 0;
  size_t num_bytes_written = 0;
  LercNS::Lerc lerc;
  
  // convert data type to proper one
  LercNS::Lerc::DataType lerc_dt = LercNS::Lerc::DT_Undefined;
  if (data_type == LercUtil::DataType::FLOAT) {
    lerc_dt = LercNS::Lerc::DT_Float;
  } else if (data_type == LercUtil::DataType::BYTE) {
    lerc_dt = LercNS::Lerc::DT_Byte;
  } else if (data_type == LercUtil::DataType::INT) {
    lerc_dt = LercNS::Lerc::DT_Int;
  } else {
    Logger::LogD("ERROR input data type %s\n", path_to_file.c_str());
    return false;
  }
  
  if (!lerc.ComputeBufferSize((void*)data,                   // raw image data, row by row, band by band
                              lerc_dt,
                              width, height, band,
                              0,                             // set 0 if all pixels are valid
                              max_z_error,                   // max coding error per pixel, or precision
                              num_bytes_needed)) {           // size of outgoing Lerc blob
    Logger::LogD("ERROR when ComputeBufferSize %s\n", path_to_file.c_str());
    return false;
  }
  
  size_t num_bytes_blob = num_bytes_needed;
  LercNS::Byte* lerc_buffer = new LercNS::Byte[num_bytes_blob];
  
  if (!lerc.Encode((void*)data,            // raw image data, row by row, band by band
                   lerc_dt,
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
  
  // clean memory
  delete[] data;
  
  return true;
}

NS_GAGO_END
