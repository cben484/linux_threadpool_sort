#pragma once

class Merge {
public:
  void merge(int *a, int beg, int med, int end);
  void merge_sort(int *a, int beg, int end);
  static int merge_two_file(char *file1, char *file2, char *output);
};