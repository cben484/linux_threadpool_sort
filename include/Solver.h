#pragma once

#include <cstdio>
class Solver {
public:
  int count_prefix(int prefix, const char *dir_path);
  static int sort_worker(char *name, int start_line, long offset, int lines,
                         char *dst_name);
  void sort_list(const char *list_name, const char *dst_dir);
  int count_lines(char *filename, FILE *filelist);
  int get_file_list(char *path, const char *filelist);
  void copy_prefix_file(const char *path, int prefix, const char *dst_name);
  void delete_directory(const char *path);
};