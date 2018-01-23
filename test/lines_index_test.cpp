#include "catch.hpp"
#include "../lines_index.h"

TEST_CASE( "LinesIndex", "[lines_index]" ) {
  //!shuffle
  LinesIndex li;
  uint64_t offs[] = 
    {
      0xff00000000, //0
      0x0012345678, //1
      0x0200400f0e, //2
      0xffffffffff, //3
      0x0000000000, //4
      0x0000000001, //5
    };
  for(uint a=0;a < sizeof(offs)/sizeof(offs[0]); a++) {
    li.addLine(offs[a]);
  }
  li.shuffle(0x7f4e95df);
  REQUIRE(li.getAt(0) == offs[0]);
  REQUIRE(li.getAt(1) == offs[5]);
  REQUIRE(li.getAt(2) == offs[2]);
  REQUIRE(li.getAt(3) == offs[4]);
  REQUIRE(li.getAt(4) == offs[1]);
  REQUIRE(li.getAt(5) == offs[3]);
  REQUIRE(li.count() == 6);
}