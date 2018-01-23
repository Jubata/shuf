#include <vector>
#include <algorithm>
#include <random>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../line_size.h"

TEST_CASE( "LineSize", "[line_size]" ) {

  FILE* f = fopen("test.bin", "w+");

  std::vector<uint8_t> input(1999);
  std::vector<uint8_t> output(10000);
  uint tests[]={123,254,255,999};
  uint tests_len[]={1,1,5,5};
  for(size_t t=0; t<sizeof(tests)/sizeof(tests[0]); t++) {
    fseek(f, 0, SEEK_SET);
    ftruncate(fileno(f), 0);

    LineSize ls;

    size_t testOffs = std::rand()%100;
    std::generate(input.begin() + testOffs, input.begin() + tests[t] + testOffs, std::rand);
    auto len8 = ls.addLine(testOffs, tests[t] + testOffs);
    ls.lineLenToTemp(len8, testOffs, f);
    REQUIRE(ftell(f) == tests_len[t]);
    
    int dummy=0xFEEDBEEF;
    fwrite(&dummy, sizeof dummy, 1, f);
    
    fseek(f, 0, SEEK_SET);
    REQUIRE(tests[t] == ls.tempToLineLen(f));

    REQUIRE(ftell(f) == tests_len[t]);
    

  }
  
  fclose(f);
  remove("test.bin");
}