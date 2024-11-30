#pragma once

class Solver {
public:
  int merge_orderd_files(int prefix, const char *dir_path);
  int count_prefix(int prefix, const char *dir_path);
};