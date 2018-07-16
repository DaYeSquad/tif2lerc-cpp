// main.cc
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

// Expect command is : ./<this_cmd> --input <folder_name_with_slash_or_tiff_name_wo_slash>
//                                  --output <folder_name_with_slash_or_tiff_name_wo_slash>
//                                  --band <band>
//                                  --maxzerror <max_z_error>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "lerc_util.h"

struct RawImage {
  uint32_t width;
  uint32_t height;
  uint32_t len;
  uint32_t band;
  gago::LercUtil::DataType data_type;
  std::vector<unsigned char> raw_data;
};

void create_directory(const char* directory) {
  struct stat st = {0};
  if (stat(directory, &st) == -1) {
    mkdir(directory, 0700);
  }
}

bool is_path_directory(const std::string& path) {
  return path.back() == '/';
}

const char *get_filename_ext(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

void write_raw_data_to_file(const struct RawImage& raw_image, const std::string& output_path) {
  FILE* fp = fopen(output_path.c_str(), "wb");
  
  fwrite(&raw_image.width, sizeof(uint32_t), 1, fp);
  fwrite(&raw_image.height, sizeof(uint32_t), 1, fp);
  fwrite(&raw_image.data_type, sizeof(int), 1, fp);
  fwrite(&raw_image.len, sizeof(uint32_t), 1, fp);
  fwrite(&raw_image.band, sizeof(uint32_t), 1, fp);
  fwrite(&raw_image.raw_data[0], sizeof(unsigned char), raw_image.raw_data.size(), fp);
  
  fclose(fp);
}

void list_files_do_stuff(const char* name, int level, const std::string& input_path,
                         const std::string& output_path, double max_z_error, int band) {
  DIR *dir;
  struct dirent *entry;
  
  if (!(dir = opendir(name)))
    return;
  if (!(entry = readdir(dir)))
    return;
  
  do {
    bool isDir = false;
#if SKR_PLATFORM==SKR_PLATFORM_MAC
    if (entry->d_type == DT_DIR) {
      isDir = true;
    }
#else
    struct stat st;
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s", name, entry->d_name);
    lstat(filename, &st);
    isDir = S_ISDIR(st.st_mode);
#endif
    
    if (isDir) {
      char path[1024];
      int len = snprintf(path, sizeof(path) - 1, "%s/%s", name, entry->d_name);
      path[len] = 0;
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      
      // create directory in dest folder.
      std::string spec_output_folder = name;
      spec_output_folder += "/";
      spec_output_folder += entry->d_name;
      spec_output_folder.replace(spec_output_folder.begin(), spec_output_folder.begin() + input_path.size(), output_path);
      create_directory(spec_output_folder.c_str());
      
      // continue
      list_files_do_stuff(path, level + 1, input_path, output_path, max_z_error, band);
    } else {
      if ((0 == strcmp("tif", get_filename_ext(entry->d_name))) ||
          (0 == strcmp("tiff", get_filename_ext(entry->d_name)))) { // allow tif and tiff extension
        // destination path
        std::string spec_output_folder = name;
        spec_output_folder += "/";
        spec_output_folder += entry->d_name;
        std::string file_path = spec_output_folder;
        
        size_t lastindex = spec_output_folder.find_last_of(".");
        std::string dest_file_name = spec_output_folder.substr(0, lastindex);
        dest_file_name += ".lerc";
        dest_file_name.replace(dest_file_name.begin(), dest_file_name.begin() + input_path.size(), output_path);
        
        bool success = gago::LercUtil::EncodeTiffOrDie(file_path,
                                                      dest_file_name,
                                                      max_z_error,
                                                      gago::LercUtil::LercVersion::V2_3,
                                                      band);
        if (!success) {
          gago::Logger::LogD("%s encode failed", file_path.c_str());
        }
      }
    }
  } while ((entry = readdir(dir)));
  closedir(dir);
}

int main(int argc, const char * argv[]) {
  std::string input_path;
  std::string output_path;
  uint32_t band = 0;
  double max_z_error = 0; // losses
  bool output_raw_data = false; // output raw data
  
  // parse input arguments
  for (int i = 0; i < argc; ++i) {
    if (0 == strcmp("--input", argv[i])) {
      input_path = argv[i + 1];
    } else if (0 == strcmp("--output", argv[i])) {
      output_path = argv[i + 1];
    } else if (0 == strcmp("--band", argv[i])) {
      band = atoi(argv[i + 1]);
    } else if (0 == strcmp("--maxzerror", argv[i])) {
      max_z_error = atof(argv[i + 1]);
    } else if (0 == strcmp("--rawdata", argv[i])) {
      output_raw_data = true;
    }
  }
  
  // give a galance
  gago::Logger::LogD("The input folder path is %s, output lerc files will be inside %s, the TIFF band is %d, max Z error given is %f",
                     input_path.c_str(),
                     output_path.c_str(),
                     band,
                     max_z_error);
  
  bool is_directory = is_path_directory(input_path);
  if (is_directory) {
    if (!is_path_directory(output_path)) {
      gago::Logger::LogD("output path should be directory (end with '/')");
      return EXIT_FAILURE;
    }
  }
  
  if (is_directory) { // loop directory recursively to covert tiffs to lercs
    // remove last slash
    // for compact with previous version implementation
    input_path = input_path.substr(0, input_path.size() - 1);
    output_path = output_path.substr(0, output_path.size() - 1);
    
    // create output directory
    create_directory(output_path.c_str());
    
    // enumerate all files in directory
    list_files_do_stuff(input_path.c_str(),
                        0,
                        input_path,
                        output_path,
                        max_z_error,
                        band);
  } else { // treat input path as file and convert tiff to lerc
    if (output_raw_data) {
      uint32_t width = 0;
      uint32_t height = 0;
      uint32_t dims = 0;
      gago::LercUtil::DataType dt;
      std::vector<unsigned char> raw_data;
      if (gago::LercUtil::ReadTiffOrDie(input_path, &width, &height, &dims, &dt, &raw_data)) {
        struct RawImage raw_image;
        raw_image.width = width;
        raw_image.height = height;
        raw_image.data_type = dt;
        raw_image.len = static_cast<uint32_t>(raw_data.size());
        raw_image.band = band;
        raw_image.raw_data = raw_data;
        
        write_raw_data_to_file(raw_image, output_path);
      }
    } else {
      gago::LercUtil::EncodeTiffOrDie(input_path,
                                      output_path,
                                      max_z_error,
                                      gago::LercUtil::LercVersion::V2_3,
                                      band);
    }
  }
  
  gago::Logger::LogD("DONE");

  return EXIT_SUCCESS;
}
