#include "Solver.h"
#include "Merge.h"
#include "ThreadPool.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <future>
#include <vector>

#define LINE_BUF_SIZE 1024
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

int Solver::merge_orderd_files(int prefix, const char *dir_path) {
  // If this is only one file prefixed with this prefix, all file has been
  // merged into this file, that is to say, the final result. return the noly
  // prefix
  if (count_prefix(prefix, dir_path) <= 1)
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