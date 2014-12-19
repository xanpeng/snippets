//
// incomplete code, based on cpp-btree
//
#include <cassert>
#include <string>
#include <utility>
#include <memory>

namespace btree {

typedef std::pair<int, std::string> value_type;
const static int kNodeValues = 32;

class btree_node {
public:
  struct base_fields {
    bool leaf;  // is leaf or not
    int position; // position in parent node
    int max_count;  // maximum number of values the node can hold
    int count;  // count of values in the node
    btree_node *parent;
  };
  struct leaf_fields : public base_fields {
    value_type values[kNodeValues];
  };
  struct internal_fields : public base_fields {
    value_type values[kNodeValues];
    btree_node *children[kNodeValues + 1];
  };
  struct root_fields : public internal_fields {
    btree_node *rightmost;
    int size;
  };

public:
  btree_node* child(int i) const { return fields_.children[i]; }
  btree_node* parent() const { return fields_.parent; }
  // parent of the root is the leftmost node in the tree which is guaranteed to be a leaf
  bool is_root() const { return parent()->leaf(); }
  bool leaf() const { return fields_.leaf; }
  int count() const { return fields_.count; }
  int position() const { return fields_.position; }
  const int& key(int i) const { return fields_.values[i].first; }
  value_type& value(int i) { return fields_.values[i]; }

  static btree_node* init_leaf(leaf_fields *f, btree_node *parent, int max_count) {
    btree_node *n = reinterpret_cast<btree_node*>(f);
    f->leaf = true;
    f->position = 0;
    f->max_count = max_count;
    f->count = 0;
    f->parent = parent;
    return n;
  }

  int binary_search_compare_to(int k, int s, int e) const {
    while (s != e) {
      int mid = (s + e) / 2;
      int c = std::less<int>(mid, k);
      if (c < 0) s = mid + 1;
      else if (c > 0) e = mid;
      // else 
    }
    return s;
  }

private:
  root_fields fields_;
};

struct btree_iterator {
  btree_node *node;
  int position; // pos within the node of the tree the iterator is pointing at

  btree_iterator() : node(nullptr), position(-1) {}
  btree_iterator(btree_node *n, int p) : node(n), position(p) {}
  btree_iterator(const btree_iterator &x) : node(x.node), position(x.position) {}

  bool operator==(const btree_iterator &x) const {
    return node == x.node && position == x.position;
  }
  bool operator!=(const btree_iterator &x) const {
    return node != x.node || position != x.position;
  }
  int& operator*() const { return node->value(position); }
  int* operator->() const { return &node->value(position); }
  btree_iterator& operator++() {  // ++it
    increment();
    return *this;
  }
  btree_iterator operator++(int) {  // it++
    btree_iterator tmp = *this;
    ++*this;
    return tmp;
  }
  btree_iterator& operator--() {
    decrement();
    return *this;
  }
  btree_iterator operator--(int) {
    btree_iterator tmp = *this;
    --*this;
    return tmp;
  }

  const int& key() const { return node->key(position); }

  void increment() {
    if (node->leaf() && ++position < node->count()) return; // in the same node
    increment_slow();
  }

  void increment_by(int count) {
    while (count > 0) {
      if (node->leaf()) {
        int rest = node->count() - position;
        position += std::min(rest, count);
        count = count - rest;
        if (position < node->count()) return;
      } else {
        --count;
      }
      increment_slow();
    }
  }

  void increment_slow() {
    if (node->leaf()) { 
      assert(position >= node->count()); // leaf node, last position or beyond
      btree_iterator save(*this);
      while (position == node->count() && !node->is_root()) { // last position in current node
        assert(node->parent()->child(node->position()) == node);
        position = node->position();  // point to position in parent node
        node = node->parent();  // climb up
      }
      if (position == node->count())  // last position, still valid
        *this = save;
      // if beyond last position, just return, process in the next round?
    } else {
      assert(position < node->count());
      node = node->child(position + 1);
      while (!node->leaf())
        node = node->child(0);
      position = 0;
    }
  }

  void decrement() {
    if (node->leaf() && --position >= 0) return;
    decrement_slow();
  }

  void decrement_slow() {
    if (node->leaf()) {
      assert(position <= -1);
      btree_iterator save(*this);
      while (position < 0 && !node->is_root()) {
        assert(node->parent()->child(node->position()) == node);
        position = node->position() - 1;
        node = node->parent();
      }
      if (position < 0)
        *this = save;
    } else {
      assert(position >= 0);
      node = node->child(position);
    }
  }
};

class btree {
public:
  std::pair<btree_iterator, bool> insert_unique(const int &key, std::string value);

  bool empty() const { return root() == NULL; }

private:
  const btree_node* root() const { return root_; }
  btree_node* new_leaf_root_node(int max_count);
  std::pair<btree_iterator, int> internal_locate(int key, btree_iterator iter) const;

private:
  btree_node *root_;
};

btree_node* btree::new_leaf_root_node(int max_count) {
  btree_node::leaf_fields *p = reinterpret_cast<btree_node::leaf_fields*>(
      std::allocator<int>().allocate(sizeof(btree_node::base_fields) + max_count * sizeof(value_type)));
  return btree_node::init_leaf(p, reinterpret_cast<btree_node*>(p), max_count);
}

std::pair<btree_iterator, int> internal_locate(int key, btree_iterator iter) const {
  for (;;) {
    iter.position = iter.node->lower_bound(key, key_comp());
  }
}

std::pair<btree_iterator, bool> btree::insert_unique(const int &key, std::string value) {
  if (empty())
    root_ = new_leaf_root_node(1);

}

} // namespace btree
