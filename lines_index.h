#include <vector>
#include <random>
#include <algorithm>

struct LinesIndex {
private:
  std::vector<uint32_t> vec32;
  std::vector<uint8_t> vec8;
public:
  void addLine(size_t begin) {
    vec32.push_back(begin);
    vec8.push_back(begin>>32);
  }

  void empty() {
    {
      std::vector<uint32_t> empty;
      std::swap(vec32, empty);
    }
    {
      std::vector<uint8_t> empty;
      std::swap(vec8, empty);
    }
  }

  void shuffle(unsigned int seed) {
    std::mt19937_64 g(seed);
    std::mt19937_64 g2(g);
    
    std::shuffle(vec32.begin(), vec32.end(), g);
    std::shuffle(vec8.begin(), vec8.end(), g2);
  }

  uint64_t getAt(size_t index) {
    return ((vec8[index] + 0ul )<<32) + vec32[index];
  }

  uint64_t count() {
    return vec32.size();
  }
};
