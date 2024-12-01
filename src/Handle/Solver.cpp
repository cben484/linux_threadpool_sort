#include "Solver.h"
#include "Merge.h"
#include "ThreadPool.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <future>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define LINE_GROUPS 100
#define LINE_BUF_SIZE 1024
#define PATH_BUF_SIZE 128
#define THREAD_NUM 4

int Solver::count_prefix(int prefix, const char *dir_path) {
  DIR *dp;
  struct dirent *ep;
  int count = 0;
  char buffer[LINE_BUF_SIZE];
  sprintf(buffer, "%d", prefix);
  size_t sz = strlen(buffer);

  dp = opendir(dir_path);
  if (dp != NULL) {
    while ((ep = readdir(dp)))
      if (!strncmp(buffer, ep->d_name,
                   sz)) // Find the file prefixed with prefix
        ++count;
    closedir(dp);
  } else {
    perror("couldn't open the directory");
  }
  return count;
}

int Solver::sort_worker(char *name, int start_line, long offset, int lines,
                        char *dst_name) {
  // 使用 std::vector 替代可变长数组
  std::vector<int> data_buffer(lines);
  char line_buffer[LINE_BUF_SIZE];
  FILE *data_file = fopen(name, "r");
  int i;

  if (data_file == NULL) {
    perror("couldn't open file");
    return -1;
  }

  // Navigate to offset
  fseek(data_file, offset, SEEK_SET);
  for (i = 0; i < lines; ++i) {
    if (!fgets(line_buffer, LINE_BUF_SIZE, data_file))
      break;
    data_buffer[i] = atoi(line_buffer);
  }

  fclose(data_file);

  Merge aa;
  // Merge sort the data chunk
  aa.merge_sort(data_buffer.data(), 0,
                i); // 使用 data() 获取指向 vector 数据的指针

  FILE *dst_file = fopen(dst_name, "w");
  if (dst_file == NULL) {
    perror("couldn't open file");
    return -1;
  }

  // Write the sorted result
  for (int j = 0; j < i; ++j) {
    fprintf(dst_file, "%d\n", data_buffer[j]);
  }

  fclose(dst_file);
  free(name);
  free(dst_name);

  return 0;
}

void Solver::sort_list(const char *list_name, const char *dst_dir) {
  ThreadPool pool(THREAD_NUM);
  FILE *list = fopen(list_name, "r");
  char line_buffer[LINE_BUF_SIZE];
  char tmp_file_name[PATH_BUF_SIZE];
  std::vector<std::future<int>> results;
  // fpos_t op;

  pool.init();

  while (fgets(line_buffer, LINE_BUF_SIZE, list)) {
    long offset = atol(strrchr(line_buffer, '\t') + 1);
    *strrchr(line_buffer, '\t') = 0;
    int start_line = atoi(strchr(line_buffer, '\t') + 1);
    *strchr(line_buffer, '\t') = 0;
    char name[PATH_BUF_SIZE];
    strcpy(name, line_buffer);
    // Get destination file name
    sprintf(tmp_file_name, "%s/0%s%d", dst_dir, strrchr(name, '/') + 1,
            start_line);
    // cout << name << " " << start_line << " " << offset << " " << LINE_GROUPS
    // << " " << tmp_file_name << endl;
    //  Sort each part and write to corresponding destination file prefix with 0
    results.emplace_back(pool.submit(sort_worker, strdup(name), start_line,
                                     offset, LINE_GROUPS,
                                     strdup(tmp_file_name)));
  }

  fclose(list);
  for (auto &&result : results)
    result.get();

  pool.shutdown();
}

int Solver::count_lines(char *filename, FILE *filelist) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("unable to open file");
    return -1;
  }

  int lines = 0;
  char buffer[LINE_BUF_SIZE];
  long offset = ftell(file);

  while (fgets(buffer, LINE_BUF_SIZE, file) != NULL) {
    if (!(lines % LINE_GROUPS)) { // Write the record every serval line
                                  // meanwhile separate the file to line chunks
      fprintf(filelist, "%s\t%d\t%ld\n", filename, lines,
              offset); // Write the record
    }
    lines++;
    offset = ftell(file); // Obtain offset of current position
  }
  fclose(file);
  return lines;
}

int Solver::get_file_list(char *path, const char *filelist) {
  DIR *dp;
  struct dirent *ep;
  char relative[PATH_BUF_SIZE];
  char real[PATH_BUF_SIZE];
  struct stat file_stat;
  FILE *fl = fopen(filelist, "w");

  dp = opendir(path);
  if (dp != NULL) {
    while ((ep = readdir(dp)))
      if (ep->d_name[0] != '.') { // Except .
        strcpy(relative, path);
        strcat(relative, "/");
        strcat(relative, ep->d_name);
        realpath(relative, real); // Get real path of the file
        stat(real, &file_stat);
        if (S_ISREG(file_stat.st_mode)) // Ignore sub directory and other file
          count_lines(real, fl);        // Start process of the file
      }
    closedir(dp);
  } else {
    perror("couldn't open the directory");
    return -1;
  }

  fclose(fl);

  return 0;
}

void Solver::copy_prefix_file(const char *path, int prefix,
                              const char *dst_name) {
  DIR *dp;
  struct dirent *ep;
  char buffer[LINE_BUF_SIZE];
  char path_buffer[PATH_BUF_SIZE];
  sprintf(buffer, "%d", prefix);
  size_t sz = strlen(buffer);
  FILE *dst_file, *src_file;

  if (!path || prefix < 0 || !dst_name)
    return;

  dp = opendir(path);
  if (!dp) {
    perror("couldn't open the directory");
    return;
  }

  dst_file = fopen(dst_name, "w");
  if (!dst_file) {
    closedir(dp);
    perror("couldn't open the file");
    return;
  }

  while ((ep = readdir(dp)))
    if (!strncmp(buffer, ep->d_name, sz)) { // Find file prefixed with prefix
      sprintf(path_buffer, "%s/", path);
      strcat(path_buffer, ep->d_name);
      src_file = fopen(path_buffer, "r");
      if (src_file) {
        while (fgets(buffer, LINE_BUF_SIZE, src_file) !=
               NULL) // Copy to file dst_name
          fprintf(dst_file, "%s", buffer);
        fclose(src_file);
      }
      break;
    }

  closedir(dp);
  fclose(dst_file);
}

void Solver::delete_directory(const char *path) {
  DIR *d = opendir(path);
  size_t path_len = strlen(path);
  int result = 0;

  if (d) {
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
      // Except . and ..
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
        char *file_path = (char *)malloc(path_len + strlen(dir->d_name) + 2);
        if (file_path) {
          snprintf(file_path, path_len + strlen(dir->d_name) + 2, "%s/%s", path,
                   dir->d_name);
          if (dir->d_type == DT_DIR) {
            // Iteratively delete sub directory
            delete_directory(file_path);
          } else {
            // Delete file
            result = remove(file_path);
            if (result != 0) {
              printf("Error deleting file: %s\n", file_path);
            }
          }
          free(file_path);
        }
      }
    }
    closedir(d);
  }
  // Delete directory
  rmdir(path);
}
