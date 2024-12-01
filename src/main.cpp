#include "Merge.h"
#include "Solver.h"
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILE_LIST_NAME "/tmp/filelist.txt"
#define TMP_FILE_DIR "/tmp/sort"
#define PATH_BUF_SIZE 128

using namespace std;

int main(int argc, char *argv[]) {
  std::cout << "C++ version: " << __cplusplus << std::endl;

  Solver s;
  Merge m;

  if (argc != 3) {
    printf("usage: sort <folder path> <destination file>\n");
    return -1;
  }

  char path[PATH_BUF_SIZE];

  strcpy(path, argv[1]);
  if (path[strlen(path) - 1] == '/')
    path[strlen(path) - 1] = 0;

  cout << "1. file list write to " << FILE_LIST_NAME << "." << endl;
  if (s.get_file_list(path, FILE_LIST_NAME)) {
    perror("get file list error");
    return -1;
  }
  cout << "2. delete tmp dir: " << TMP_FILE_DIR << "." << endl;
  s.delete_directory(TMP_FILE_DIR);
  cout << "3. create tmp dir: " << TMP_FILE_DIR << "." << endl;
  mkdir(TMP_FILE_DIR, 0777);
  cout << "4. sort file chunk in " << TMP_FILE_DIR << "." << endl;
  s.sort_list(FILE_LIST_NAME, TMP_FILE_DIR);
  cout << "5. sort partial sorted file chunk." << endl;
  int result_prefix = m.merge_orderd_files(0, TMP_FILE_DIR);
  cout << "6. copy result to " << argv[2] << "." << endl;
  s.copy_prefix_file(TMP_FILE_DIR, result_prefix, argv[2]);
  return 0;
}
