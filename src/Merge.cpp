#include "Merge.h"
#include "Solver.h"
#include "ThreadPool.h"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <future>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define FILE_LIST_NAME "/tmp/filelist.txt"
#define TMP_FILE_DIR "/tmp/sort"
#define LINE_GROUPS 100
#define THREAD_NUM 4
#define LINE_BUF_SIZE 1024
#define PATH_BUF_SIZE 128

void Merge::merge(int *a, int beg, int med, int end) {
  int left = beg, right = med;
  size_t size = end - beg;     // size 是运行时计算的
  std::vector<int> temp(size); // 使用 vector 替代静态数组
  int i = 0;

  while (left < med || right < end) {
    if (right >= end || ((left < med && right < end) && (a[left] < a[right])))
      temp[i++] = a[left++];
    else
      temp[i++] = a[right++];
  }

  std::memcpy(a + beg, temp.data(),
              size * sizeof(int)); // 使用 vector 的 data() 获取底层数据
}

void Merge::merge_sort(int *a, int beg, int end) {
  Merge aa;
  if (end - beg <= 1)
    return;
  int med = beg + (end - beg) / 2;
  merge_sort(a, beg, med);
  merge_sort(a, med, end);
  aa.merge(a, beg, med, end);
}

int Merge::merge_two_file(char *file1, char *file2, char *output) {
  char line_buffer1[LINE_BUF_SIZE];
  char line_buffer2[LINE_BUF_SIZE];
  FILE *f1 = fopen(file1, "r");
  FILE *f2 = fopen(file2, "r");
  FILE *fo = fopen(output, "w");

  if (!fo || (!f1 && !f2)) {
    perror("couldn't open file");
    if (fo)
      fclose(fo);
    if (f1)
      fclose(f1);
    if (f2)
      fclose(f2);
    return -1;
  }

  if (!f2) {
    while (fgets(line_buffer1, LINE_BUF_SIZE, f1))
      fprintf(fo, "%s", line_buffer1);
    fclose(f1);
    fclose(fo);
    return 0;
  }

  if (!f1) {
    while (fgets(line_buffer1, LINE_BUF_SIZE, f2))
      fprintf(fo, "%s", line_buffer1);
    fclose(f2);
    fclose(fo);
    return 0;
  }

  char *s1 = fgets(line_buffer1, LINE_BUF_SIZE, f1);
  char *s2 = fgets(line_buffer2, LINE_BUF_SIZE, f2);

  while (s1 || s2) { // Ordinary merge operation
    if (!s2 || (s1 && s2 && (atoi(line_buffer1) <= atoi(line_buffer2)))) {
      fprintf(fo, "%s", line_buffer1);
      s1 = fgets(line_buffer1, LINE_BUF_SIZE, f1);
    } else {
      fprintf(fo, "%s", line_buffer2);
      s2 = fgets(line_buffer2, LINE_BUF_SIZE, f2);
    }
  }

  fclose(f1);
  fclose(f2);
  fclose(fo);
  free(file1);
  free(file2);
  free(output);

  return 0;
}
