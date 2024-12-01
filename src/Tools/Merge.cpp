#include "Merge.h"
#include "Solver.h"
#include "ThreadPool.h"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <future>
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

int Merge::merge_orderd_files(int prefix, const char *dir_path) {
  // If this is only one file prefixed with this prefix, all file has been
  // merged into this file, that is to say, the final result. return the noly
  // prefix
  Solver s;
  if (s.count_prefix(prefix, dir_path) <= 1)
    return prefix;

  DIR *dp;
  struct dirent *ep;
  std::vector<std::future<int>> results;
  char file1[LINE_BUF_SIZE], file2[LINE_BUF_SIZE], output_file[LINE_BUF_SIZE];
  char buffer[LINE_BUF_SIZE];
  int i = 1;
  ThreadPool pool(THREAD_NUM);

  pool.init();
  dp = opendir(dir_path);
  sprintf(buffer, "%d", prefix);
  size_t sz = strlen(buffer);

  if (dp != NULL) {
    while ((ep = readdir(dp))) {
      if (!strncmp(buffer, ep->d_name, sz)) {
        if (i % 2) { // Mark the first file
          sprintf(file1, "%s/%s", dir_path, ep->d_name);
        } else { // Mark the second file
          sprintf(file2, "%s/%s", dir_path, ep->d_name);
          sprintf(output_file, "%s/%d%s", dir_path, prefix + 1,
                  strrchr(file1, '/') + sz + 1);
          // cout << file1 << ":" << file2 << ":" << output_file << endl;
          //  Merge the first and second file marked before, add the thread to
          //  thread pool
          results.emplace_back(pool.submit(Merge::merge_two_file, strdup(file1),
                                           strdup(file2), strdup(output_file)));
        }
        ++i;
      }
    }
    if (i % 2 == 0) {
      sprintf(output_file, "%s/%d%s", dir_path, prefix + 1,
              strrchr(file1, '/') + sz + 1);
      // cout << file1 << "::" << output_file << endl;
      //  If this is a single file, fill another parameter of merge_two_file
      //  with NULL
      results.emplace_back(pool.submit(Merge::merge_two_file, strdup(file1),
                                       (char *)NULL, strdup(output_file)));
    }
    for (auto &&result : results)
      result.get(); // Wait for all sort threads to complete
    closedir(dp);
  } else {
    perror("couldn't open the directory");
    return -1;
  }

  // Close pool of this sort
  pool.shutdown();
  // Sort operation of this prefix complete, start the sort for next prefix
  return merge_orderd_files(prefix + 1, dir_path);
}