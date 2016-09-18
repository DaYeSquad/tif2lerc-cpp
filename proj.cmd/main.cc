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

// Expect command is : ./<this_cmd> --input <folder_name> --output <folder_name> --band <band> --maxzerror <max_z_error>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "lerc_util.h"

void create_directory(const char* directory) {
  gago::Logger::LogD("create directory %s", directory);
  struct stat st = {0};
  if (stat(directory, &st) == -1) {
    mkdir(directory, 0700);
  }
}

const char *get_filename_ext(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

void list_files_do_stuff(const char* name, int level, const std::string& input_path,
                         const std::string& output_path, double max_z_error, int band,
                         bool signed_type) {
  gago::Logger::LogD("DEBUG - working %s", name);
  
  DIR *dir;
  struct dirent *entry;
  
  if (!(dir = opendir(name)))
    return;
  if (!(entry = readdir(dir)))
    return;
  
  gago::Logger::LogD("DEBUG - step1 - working %s", name);
  
  do {
    if (entry->d_type == DT_DIR) {
      gago::Logger::LogD("DEBUG - step2/ DIR - working %s", name);
      
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
      list_files_do_stuff(path, level + 1, input_path, output_path, max_z_error, band, signed_type);
    } else {
      gago::Logger::LogD("DEBUG - step2/ FILE - working %s", name);
      
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
                                                      gago::LercUtil::DataType::UNKNOWN,
                                                      band,
                                                      signed_type);
        if (!success) {
          gago::Logger::LogD("%s encode failed", file_path.c_str());
        }
      }
    }
  } while ((entry = readdir(dir)));
  closedir(dir);
}

int main(int argc, const char * argv[]) {
  std::string input_folder_path;
  std::string output_folder_path;
  uint32_t band = 0;
  double max_z_error = 0; // losses
  bool signed_type = false;
  
  // parse input arguments
  for (int i = 0; i < argc; ++i) {
    if (0 == strcmp("--input", argv[i])) {
      input_folder_path = argv[i + 1];
    } else if (0 == strcmp("--output", argv[i])) {
      output_folder_path = argv[i + 1];
    } else if (0 == strcmp("--band", argv[i])) {
      band = atoi(argv[i + 1]);
    } else if (0 == strcmp("--maxzerror", argv[i])) {
      max_z_error = atof(argv[i + 1]);
    } else if (0 == strcmp("--signed", argv[i])) {
      if (0 == strcmp("true", argv[i])) {
        signed_type = true;
      }
    }
  }
  
  // give a galance
  gago::Logger::LogD("The input folder path is %s, output lerc files will be inside %s, the TIFF band is %d, max Z error given is %f",
                     input_folder_path.c_str(),
                     output_folder_path.c_str(),
                     band,
                     max_z_error);
  
  // create output directory
  create_directory(output_folder_path.c_str());
  
  // enumerate all files in directory
  list_files_do_stuff(input_folder_path.c_str(),
                      0,
                      input_folder_path,
                      output_folder_path,
                      max_z_error,
                      band,
                      signed_type);
  
  gago::Logger::LogD("DONE");
  
  return EXIT_SUCCESS;
}
