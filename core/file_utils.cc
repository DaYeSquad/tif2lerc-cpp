/*
 * FileUtils.cpp
 *
 *  Created on: 2013-9-11
 *      Author: FrankLin
 */

#include "file_utils.h"

#include <sys/stat.h>
#include <vector>
#include <algorithm>

#if (SKR_PLATFORM == SKR_PLATFORM_MAC)
#include <ftw.h>
#endif

NS_GAGO_BEGIN

////////////////////////////////////////////////////////////////////////////////
// FileUtils, C:

#if (SKR_PLATFORM == SKR_PLATFORM_IOS) || (SKR_PLATFORM == SKR_PLATFORM_MAC)
static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  auto ret = remove(fpath);
  if (ret) {
  }
  
  return ret;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// FileUtils, public:

// Creation and lifetime --------------------------------------------------------

FileUtils *FileUtils::s_shared_fileutil_ = nullptr;

FileUtils::FileUtils() {
}

FileUtils::~FileUtils() {
}

void FileUtils::PurgeFileUtils() {
  SKR_SAFE_DELETE(s_shared_fileutil_);
}

// Utilities --------------------------------------------------------

unsigned char *FileUtils::GetFileData(const char *filename, const char *mode,
                                      unsigned long *out_size) {
  unsigned char *p_data = 0;
  
  if ((!filename) || (!mode) || 0 == strlen(filename)) {
    return 0;
  }
  
  do {
    FILE *fp = fopen(filename, mode);
    SKR_BREAK_IF(!fp);
    
    unsigned long size;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    p_data = new unsigned char[size + 1];
    size = fread(p_data, sizeof(unsigned char), size, fp);
    fclose(fp);
    
    p_data[size] = '\0';
    
    if (out_size) {
      *out_size = size;
    }
  }
  while (0);
  
  return p_data;
}

bool FileUtils::CreateDirectory(const std::string& path) {
  SKR_ASSERT(!path.empty());
  
  if (IsDirectoryExist(path))
    return true;
  
  // Split the path
  size_t start = 0;
  size_t found = path.find_first_of("/\\", start);
  std::string subpath;
  std::vector<std::string> dirs;
  
  if (found != std::string::npos)
  {
    while (true)
    {
      subpath = path.substr(start, found - start + 1);
      if (!subpath.empty())
        dirs.push_back(subpath);
      start = found+1;
      found = path.find_first_of("/\\", start);
      if (found == std::string::npos)
      {
        if (start < path.length())
        {
          dirs.push_back(path.substr(start));
        }
        break;
      }
    }
  }

  DIR *dir = NULL;
  
  // Create path recursively
  subpath = "";
  for (int i = 0; i < dirs.size(); ++i) {
    subpath += dirs[i];
    dir = opendir(subpath.c_str());
    if (!dir)
    {
      int ret = mkdir(subpath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
      if (ret != 0 && (errno != EEXIST))
      {
        return false;
      }
    }
  }
  return true;
}

bool FileUtils::IsDirectoryExist(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) == 0)
  {
    return S_ISDIR(st.st_mode);
  }
  return false;
}

NS_GAGO_END
