#include <vector>
#include <stdlib.h>
#include <memory>
#include <list>
#include <algorithm>
#include <cassert>

#include <iostream>

struct RandomMergeFeeder {
  RandomMergeFeeder(const std::vector<size_t> &segments);
  size_t get();
private:

  struct RMFNode {
    size_t pool;
    size_t sum;
    std::unique_ptr<RMFNode> left;
    std::unique_ptr<RMFNode> right;
    RMFNode(size_t pool,
            size_t sum, 
            std::unique_ptr<RMFNode> left, 
            std::unique_ptr<RMFNode> right) : 
      pool(pool), sum(sum), left(std::move(left)), right(std::move(right)) 
    {}
    RMFNode(RMFNode&& other) = default;
    RMFNode& operator=(RMFNode&&) = default;
    // ~RMFNode() {std::cout<<"destroyed "<<(void*)this<<"\n";}
  };

  struct Request {
    size_t value;
    RMFNode* node;
  };

  std::unique_ptr<RMFNode> head;

  std::random_device rd;
  public:
  std::mt19937 urbg;
};


RandomMergeFeeder::RandomMergeFeeder(const std::vector<size_t> &segments) :
    urbg(rd()) {
  assert(!segments.empty());

  //construct segments sum tree
  std::vector<std::unique_ptr<RMFNode> > level;
  level.reserve(segments.size());
  auto end = segments.end();
  size_t pool = 0;
  for(auto segment=segments.begin(); segment != end; segment++, pool++) {
    level.push_back( std::make_unique<RMFNode>(pool, *segment, nullptr, nullptr) );
  }
  while(level.size()>1) {
    std::vector<std::unique_ptr<RMFNode> > level2;
    
    auto end = level.end();
    for(auto it=level.begin(); it != end; it++) {
      auto it2 = it++;
      if(it != end) {
        level2.push_back( std::make_unique<RMFNode>(
          (size_t)-1,
          (*it)->sum + (*it2)->sum,
          std::move(*it),
          std::move(*it2) ) );
      } else {
        level2.push_back( std::move(*it2) );
        break;
      }
    }
    std::swap(level, level2);
  }
  head = std::move(level[0]);
}

size_t RandomMergeFeeder::get() {
  RMFNode *parent = nullptr;
  RMFNode *sibling = nullptr;
  RMFNode *node = head.get();
  assert(node->sum);
  while (1) {
    size_t pool = node->pool;
    if (node->sum == 1 && head.get()!=node) {
      std::swap(*parent, *sibling);

      //Now sibling has circular dependency on itself. We need to destroy it carefully
      if(sibling->left.get() == sibling)
        sibling->left.reset();
      else
        sibling->right.reset();
      return pool;
    }
    if(pool!=(size_t)-1) {
      node->sum--;
      return pool;
    }

    typedef typename std::uniform_int_distribution<size_t> __distr_type;
    typedef typename __distr_type::param_type __p_type;
    __distr_type __d;
    size_t value = __d(urbg, __p_type(0, node->sum-1));

    // size_t value = double(urbg()) * node->sum / urbg.max();
    node->sum--;
    RMFNode* left = node->left.get();
    RMFNode* right = node->right.get();
    parent = node;
    if (value >= right->sum) {
      node = left;
      sibling = right;
    } else {
      node = right;
      sibling = left;
    }
  }
}