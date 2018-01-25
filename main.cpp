#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <bitset>
#include <malloc.h>
#include <ctime>
#include <cstring>

#include "random_merge_tree.h"
#include "lines_index.h"
#include "line_size.h"
#include "temp_files.h"

using namespace std;

struct Shuffler {
  Shuffler(uint64_t maxmem): maxmem(maxmem) {};
  int readFile(const char* fname);

 private:
  uint64_t maxmem;
  LineSize lineSize;
  vector<unsigned char> fileBuf;
  LinesIndex linesIndex;
  TempFiles tempFiles;
};


int Shuffler::readFile(const char* fname) {
  FILE* f = fopen(fname, "rb");
  if (!f)
    return 1;


  fileBuf.resize(maxmem);
  
  size_t prefix = 1;

  while (!feof(f)) {
    fputs("reading input file\n", stderr);
    
    clock_t c1 = clock();

    size_t read = std::fread(&fileBuf[prefix], 1, fileBuf.size() - prefix, f);

    clock_t c2 = clock();

    double elapsed_secs =  double(c2 - c1) / CLOCKS_PER_SEC;
    std::cerr << "mbytes/s "<< (read / elapsed_secs / 1048576.0 ) << "\n";
    
    size_t bufSize = prefix + read;

    fputs("indexing\n", stderr);
    size_t start = 0;

    uint8_t* ptr = (uint8_t*)memchr(&fileBuf[0] + prefix, '\n', bufSize - prefix); //bufSize
    while(ptr) {
      auto end = ptr - &fileBuf[0];
      uint8_t lenByte = lineSize.addLine(start+1, end); //exclude leading '\n'
      linesIndex.addLine(start); //include leading len8 byte
      fileBuf[start] = lenByte; //overwrite leading len8 byte
      start = end;
      ptr = (uint8_t*)memchr(&fileBuf[0] + end + 1, '\n', bufSize - end - 1); //bufSize
    }

    clock_t c3 = clock();
    elapsed_secs =  double(c3 - c2) / CLOCKS_PER_SEC;
    std::cerr << "mbytes/s "<< (read / elapsed_secs / 1048576.0 ) << "\n";

    //last line doesn't have trailing '\n'. So add the line manually
    if(feof(f)) {
      uint8_t lenByte = lineSize.addLine(start+1, bufSize);
      linesIndex.addLine(start);
      fileBuf[start] = lenByte;
    }

    fputs("shuffling\n", stderr);
    std::random_device rd;
    linesIndex.shuffle(rd());
    
    fputs("writing to temp file\n", stderr);
    uint64_t count = linesIndex.count();
    FILE* fTemp = tempFiles.createTempFile(count);
    for (size_t i =0; i<count; i++) {
      uint64_t offset = linesIndex.getAt(i);
      size_t len = lineSize.lineLenToTemp(fileBuf[offset], offset+1, fTemp);
      fwrite(&fileBuf[offset + 1], 1, len, fTemp);
    }

    linesIndex.empty();
    lineSize.empty();
    prefix = bufSize - start;
    memcpy(&fileBuf[0], &fileBuf[0] + start, prefix);//copy partial last line in buffer to the beginning
  }
  
  fputs("merging to output file\n", stderr);
  tempFiles.seekAll(0, SEEK_SET);

  vector<char> lineBuf(255);
  RandomMergeFeeder rmf(tempFiles.linesCount);
  uint64_t total=tempFiles.getTotalLines();
  for(size_t i=0;i<total;i++) {
    size_t pool = rmf.get();
    FILE *fTemp = tempFiles.getFileAt(pool);
    auto len = lineSize.tempToLineLen(fTemp);
    lineBuf.resize(len+1);
    fread(&lineBuf[0], 1, len, fTemp);
    lineBuf[len]='\n';
    
    //last line does not include trailing '\n'
    if(i==total-1)
      lineBuf.resize(lineBuf.size()-1);

    fwrite(&lineBuf[0], 1, lineBuf.size(), stdout);
  }

  return 0;
}

int main(int argc, char* argv[]) {
  atexit (TempFiles::cleanup);

  if (argc == 3) {
    Shuffler shuffler(atol(argv[2]));
    if (-1 == shuffler.readFile(argv[1])) {
      exit(EXIT_FAILURE);
    }
  }
}