// lerc.h
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

#ifndef LERC_CORE_LERC_UTIL_H_
#define LERC_CORE_LERC_UTIL_H_

#include <string>

#include "macros.h"
#include "logger.h"

NS_GAGO_BEGIN

/// Esri lerc format utility tool, current version is Lerc2 v3.
///
/// @since 0.1
///
class LercUtil {
public:
  
  // Lerc version, current is lerc2 v3
  // it is only for future consideration
  enum class LercVersion {
    DEFAULT = 0,
    V2_3    = 1
  };
  
  enum class DataType {
    UNKNOWN = -1,
    FLOAT = 0,
    BYTE = 1,
    INT = 2,
  };
  
  // TIFF --------------------------------------------------------
  
  /**
   *  Encode TIFF to Lerc (lerc2 v3).
   *
   *  @param path_to_file Input TIFF path.
   *  @param output_path  Output LERC path.
   *  @param max_z_error  Max Z error defined in LERC.
   *  @param lerc_ver     LERC version number, only supports V2_3 right now.
   *  @param data_type    Can be FLOAT or BYTE.
   *  @param band         Band of TIFF, grayscale is 1, RGB is 3 and RGBA is 4.
   *
   *  @return Returns false if encodes failed.
   */
  static bool EncodeTiffOrDie(const std::string& path_to_file, const std::string& output_path,
                              double max_z_error, LercVersion lerc_ver, DataType data_type,
                              uint16_t band);
  
private:
  
  // Creation and lifetime --------------------------------------------------------
  
  LercUtil() {};
  virtual ~LercUtil() {}
  
  
  DISALLOW_COPY_AND_ASSIGN(LercUtil);
};

NS_GAGO_END

#endif /* LERC_CORE_LERC_UTIL_H_ */
