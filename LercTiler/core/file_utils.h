// FileUtils
// Inspired by cocos2d-x
// @author Frank Lin (lin.xiaoe.f@gmail.com)
//
// Copy from sakura, build for maxOS and linux.
//

#ifndef SAKURA_FILE_UTILS_H_
#define SAKURA_FILE_UTILS_H_

#include <string>
#include <vector>

#include "macros.h"

NS_GAGO_BEGIN

/// Singleton file utils.
class FileUtils {
public:
  
  // Creation and lifetime --------------------------------------------------------
  
  FileUtils();
  virtual ~FileUtils();
  
  // shared instance
  static FileUtils* SharedFileUtils();
  static void PurgeFileUtils();
  
  // Utils --------------------------------------------------------
  
  virtual unsigned char* GetFileData(const char* filename, const char* mode,
                                     unsigned long* out_size);
  
  virtual bool IsDirectoryExist(const std::string& path);
  
  virtual bool CreateDirectory(const std::string& path);
  
  // Utils should be override on each platform --------------------------------------------------------
  
  virtual bool IsFileExist(const std::string& path) = 0;
  
protected:
  static FileUtils* s_shared_fileutil_;
  
private:
  DISALLOW_COPY_AND_ASSIGN(FileUtils);
};

NS_GAGO_END

#endif /* SAKURA_FILE_UTILS_H_ */
