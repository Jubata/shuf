#include <unordered_map>
#include <limits>

struct LineSize {
  std::unordered_map<uint64_t, uint32_t> line2size;//actual line length is line2size[x] + 254 bigger

  //begin points to first line character
  //returns len8 
  uint8_t addLine(size_t begin, size_t end) {
    size_t len = end - begin;
    if (len>254) {
      len = len - 254;
      if(len > std::numeric_limits<uint32_t>::max()) {
        fputs("line is bigger then 2^32-1", stderr);
        exit(3);
      }
      line2size[begin] = len;
      return 255;
    } else {
      return len;
    }
  }

  //Writes temp coded line length
  //offset - point to first line character
  //returns string lentgth in bytes
  size_t lineLenToTemp(uint8_t len8, uint64_t offset, FILE *f) {
    fwrite(&len8, sizeof len8, 1, f);
    if(len8 == 255) {
      uint32_t len = line2size[offset];
      fwrite(&len, sizeof len, 1, f);
      return 254+(uint64_t)len;
    } else {
      return len8;
    }
  }

  static uint64_t tempToLineLen(FILE *f) {
    uint8_t len8;
    fread(&len8, sizeof len8, 1, f);
    if(len8 < 255) {
      return len8;
    } else {
      uint32_t len32;
      fread(&len32, sizeof len32, 1, f);
      return 254 + (uint64_t)len32;
    }
  }

  void empty() {
    std::unordered_map<uint64_t, uint32_t> empty;
    std::swap(line2size, empty);
  }
};