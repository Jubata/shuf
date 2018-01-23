#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <algorithm>

struct TempFiles {

  std::vector<FILE*> tempFiles;
  std::vector<uint64_t> linesCount;

  FILE* createTempFile(uint64_t linesCount) {
    char tmpl[PATH_MAX] = "tmp/shuffleXXXXXX";
    int descr = mkstemp(tmpl);
    FILE* f = fdopen(descr, "w+");
    tempFiles.push_back(f);
    TempFiles::linesCount.push_back(linesCount);
    return f;
  }
  
  size_t count() {
    return tempFiles.size();
  }

  FILE* getFileAt(size_t index) {
    return tempFiles[index];
  }

  uint64_t getTotalLines() {
    return std::accumulate(linesCount.begin(), linesCount.end(), 0);
  }

  void seekAll(long int off, int whence) {
    for(auto f: tempFiles) {
      fseek(f, off, whence);
    }
  }
};