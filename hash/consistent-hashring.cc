//
// Hash functions: https://www.byvoid.com/blog/string-hash-compare
//

#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <utility>

namespace chash { // consistent hash

typedef std::pair<std::string, std::set<std::string>> Node;
class HashRing {
public:
  typedef std::map<int, Node> NodeMap;

  HashRing(unsigned int replicas) : replicas_(replicas) {}

  int AddNode(const Node& node) {
    int hash;
    std::string node_key = node.first;
    for (int r = 0; r < replicas_; ++r) {
      hash = SdbmHash((node_key + std::to_string(r)).c_str());
      ring_[hash] = node;
    }
    return hash;
  }

  void RemoveNode(const Node& node) {
    std::string node_key = node.first;
    for (int r = 0; r < replicas_; ++r) {
      int hash = SdbmHash((node_key + std::to_string(r)).c_str());
      ring_.erase(hash);
    }
  }

  const Node& GetNode(const std::string& key) {
    assert(!ring_.empty());

    int hash = SdbmHash(key.c_str());
    NodeMap::const_iterator it = ring_.lower_bound(hash);
    if (it == ring_.end())
      it = ring_.begin(); // wrap around
    return it->second;
  }

private:
  int SdbmHash(const char* str) {
    int hash = 0;
    int c;
    while ((c = *str++)) {
      hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
  }

  NodeMap ring_;
  const int replicas_;
};

} // namespace chash

//
// g++ consistent-hashring.cc --std=c++11 -g -Wall -o chash
//
int main() {
  chash::HashRing ring(2);
  ring.AddNode(std::make_pair("cache1.example.com", std::set<std::string>()));
  ring.AddNode(std::make_pair("cache2.example.com", std::set<std::string>()));
  ring.AddNode(std::make_pair("cache3.example.com", std::set<std::string>()));

  std::vector<std::string> fruits = {"culinary", "lilium", "apple", "pear", "banana", "orange", };
  // write
  for (std::string f : fruits) {
    chash::Node& node = const_cast<chash::Node&>(ring.GetNode(f));
    std::cout << "Storing " << f << " on node " << node.first << std::endl;
    node.second.insert(f);
  }

  // read
  for (std::string f : fruits) {
    chash::Node& node = const_cast<chash::Node&>(ring.GetNode(f));
    std::cout << "Found " << f << " on server " << node.first << std::endl;
  }

  return 0;
}
