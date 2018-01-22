#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <malloc.h>

#include "random_merge_tree.h"
//-std=c++14

using namespace std;

vector<FILE*> tempFiles;

int getTemp() {
  static char tmpl[] = "tmp/shuffleXXXXXX";
  char fname[PATH_MAX];

  strcpy(fname, tmpl);
  return mkstemp(fname);
}

template <typename T>
inline void freeContainer(T& p_container) {
  T empty;
  using std::swap;
  swap(p_container, empty);
}

struct LinesIndex {
private:
  vector<uint32_t> vec32;
  vector<uint8_t> vec8;
public:
  void addLine(size_t begin) {
    vec32.push_back(begin);
    vec8.push_back(begin>>32);
  }

  void empty() {
    freeContainer(vec32);
    freeContainer(vec8);
  }

  void shuffle() {

    std::random_device rd;
    std::mt19937_64 g(rd());
    std::mt19937_64 g2(g);
    
    std::shuffle(vec32.begin(), vec32.end(), g);
    std::shuffle(vec32.begin(), vec32.end(), g2);
  }

  uint64_t getAt(size_t index) {
    return ((vec8[index] + 0ul )<<32) + vec32[index];
  }

  uint64_t count() {
    return vec32.size();
  }
};

struct Shuffler {
  Shuffler(uint64_t maxmem): maxmem(maxmem) {};
  int readFile(const char* fname);

 private:
  uint64_t maxmem;
  unordered_map<uint32_t, uint32_t> line2size;
  vector<unsigned char> fileBuf;
  LinesIndex linesIndex;

  void addLine(size_t start, size_t end);
};

void Shuffler::addLine(size_t begin, size_t end) {
  linesIndex.addLine(begin);
  size_t len = end - begin - 1;
  if (end - begin > 254) {
    line2size[begin] = end - begin;
    fileBuf[begin] = 255;
  } else {
    fileBuf[begin] = len;
  }
}

int Shuffler::readFile(const char* fname) {
  FILE* f = fopen(fname, "rb");
  if (!f)
    return 1;


  fileBuf.resize(maxmem);

  while (!feof(f)) {
    size_t read = std::fread(&fileBuf[1], 1, fileBuf.size() - 1, f);

    size_t start = 0;
    for (size_t i = 1; i < read; i++) {
      if (fileBuf[i] == '\n') {
        addLine(start, i);
        start = i;
      }
    }
    if(feof(f) && start!=read) {
      addLine(start, read);
      start = read;
    }

    linesIndex.shuffle();

    int tempDescriptor = getTemp();
    FILE* fTemp = fdopen(tempDescriptor, "w+");
    tempFiles.push_back(fTemp);

    uint64_t count = linesIndex.count();
    fwrite(&count, sizeof count, 1, fTemp);
    for (size_t i =0; i<count; i++) {
      uint64_t offset = linesIndex.getAt(i);
      if (fileBuf[offset] == 255) {
        fwrite(&fileBuf[offset], 1, 1, fTemp);
        uint32_t len = line2size[offset];
        fwrite(&len, 1, sizeof len, fTemp);
        fwrite(&fileBuf[offset + 1], 1, len, fTemp);
      } else {
        uint32_t len = fileBuf[offset];
        fwrite(&fileBuf[offset], 1, len + 1, fTemp);
      }
    }

    linesIndex.empty();
    freeContainer(line2size);
  }

  vector<size_t> segments;
  size_t total=0;
  for(auto it=tempFiles.begin(); it!=tempFiles.end(); it++) {
    FILE *fTemp = *it;
    fseek(fTemp, 0, SEEK_SET);
    uint64_t size;
    fread(&size, sizeof size, 1, fTemp);
    segments.push_back(size);
    total+=size;
  }

  vector<char> lineBuf(255);
  RandomMergeFeeder rmf(segments);
  for(size_t i=0;i<total;i++) {
    size_t pool = rmf.get();
    FILE *fTemp = tempFiles[pool];
    unsigned char len;
    fread(&len, sizeof len, 1, fTemp);
    if(feof(fTemp))
      return 2;
    if(len == 255) {
      uint32_t len;
      fread(&len, sizeof len, 1, fTemp);
      lineBuf.resize(len+1);
      fread(&lineBuf[0], 1, len, fTemp);
      lineBuf[len]='\n';
    } else {
      lineBuf.resize(len+1);
      fread(&lineBuf[0], 1, len, fTemp);
      lineBuf[len]='\n';
    }
    fwrite(&lineBuf[0], 1, lineBuf.size(), stdout);
  }

  return 0;
}

int main(int argc, char* argv[]) {
 
  if (argc == 3) {
    Shuffler shuffler(atol(argv[2]));
    if (-1 == shuffler.readFile(argv[1])) {
      exit(EXIT_FAILURE);
    }
  }
}