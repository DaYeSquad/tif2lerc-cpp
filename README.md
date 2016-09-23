## OVERVIEW

Tif2lerc gives ability to convert tif file to esri lerc format (lerc2 v3) on macOS and linux, loses all tags.


## BUILD & INSTALL

```shell
cd <root_dir>
cmake .
make install
```

The default install path is in ./bin folder.


## RUN

1. Open terminal
2. ./lerctiler --input <path_to_tiff_folder> --output <path_to_output_folder> --band <band_as_int> --maxzerror <max_z_error>


## RAW DATA

1. Open terminal
2. ./lerctiler --input <path_to_tiff_folder> --output <path_to_output_folder> --band <band_as_int> --maxzerror <max_z_error> --rawdata
