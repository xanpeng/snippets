//
// not working!
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct btree_node_struct {
  struct btree_node_struct *parent;
  struct btree_node_struct *left;
  struct btree_node_struct *right;
} btree_node;

typedef struct btree_struct {
  btree_node *root;
  int node_size;
  int elem_size;
  int (*cmp)(const void *, const void *);
} btree;

// helper function declaration
static btree_node* btree_node_search(btree* tree, btree_node* node, void *key);
static btree_node* btree_node_minimum(btree_node *node);
static btree_node* btree_node_maximum(btree_node *node);
static btree_node* btree_node_successor(btree_node *node);
static btree_node* btree_node_make(btree *tree, void *data);
static void btree_node_close(btree_node *node);
void btree_really_print(btree *tree, btree_node *node, FILE *where, unsigned int spaces);

static inline void* data_get(btree *tree, btree_node *node) {
  return (char*) node + tree->node_size;
}
static inline void data_copy(btree *tree, void *dest, void *src) {
  memcpy(dest, src, tree->elem_size);
}

btree* btree_create(int size, int (*cmp)(const void *, const void *)) {
  btree* tree = (btree*) malloc(sizeof(btree));
  if (tree) {
    tree->root = NULL;
    tree->node_size = sizeof(btree_node);
    tree->elem_size = size;
    tree->cmp = cmp;
  }
  return tree;
}


int btree_search(btree* tree, void *key, void *ret) {
  btree_node* node = tree->root;
  node = btree_node_search(tree, node, key);
  if (node) {
    if (ret) data_copy(tree, ret, data_get(tree, node));
    return 0;
  }
  else return 1;
}

int btree_minimum(btree *tree, void *ret) {
  btree_node *node = btree_node_minimum(tree->root);
  if (node) {
    if (ret) data_copy(tree, ret, data_get(tree, node));
    return 0;
  }
  return 1;
}

int btree_maximum(btree *tree, void *ret) {
  btree_node *node = btree_node_maximum(tree->root);
  if (node) {
    if (ret) data_copy(tree, ret, data_get(tree, node));
    return 0;
  }
  return 1;
}

int btree_empty(btree *tree) {
  return tree->root ? 0 : 1;
}

int btree_successor(btree *tree, void *key, void *ret) {
  if (!tree->root) return 1;

  btree_node *node = btree_node_search(tree, tree->root, key);
  if (!node) return 1;

  node = btree_node_successor(node);
  if (node) {
    if (ret) data_copy(tree, ret, data_get(tree, node));
    return 0;
  }
  return 1;
}

int btree_predecessor(btree *tree, void *key, void *ret) {
  if (!tree->root) return 1;

  btree_node *node = btree_node_search(tree, tree->root, key);
  if (!node) return 1;

  if (node->left) {
    node = btree_node_maximum(node->left);
  } else {
    btree_node *parent = node->parent;
    while (parent && node == parent->left) {
      node = parent;
      parent = node->parent;
    }
    node = parent;
  }

  if (node) {
    if (ret) data_copy(tree, ret, data_get(tree, node));
    return 0;
  }
  return 1;
}

int btree_insert(btree *tree, void *data) {
  btree_node *parent, *node, *newnode;
  newnode = btree_node_make(tree, data);
  if (!newnode) return 1;

  node = tree->root;
  while (node) {
    parent = node;
    if (tree->cmp(data, data_get(tree, node)) < 0)
      node = node->left;
    else
      node = node->right;
  }

  newnode->parent = parent;
  if (!parent) {
    tree->root = newnode;
  } else {
    if (tree->cmp(data, data_get(tree, parent)) < 0)
      parent->left = newnode;
    else
      parent->right = newnode;
  }
  return 0;
}

int btree_delete(btree *tree, void *data) {
  btree_node *node, *remove, *other;
  if (!tree->root) return 1;

  node = btree_node_search(tree, tree->root, data);
  if (!node) return 1;

  if (!node->left || !node->right)
    remove = node;
  else
    remove = btree_node_successor(node);

  if (remove->left)
    other = remove->left;
  else
    other = remove->right;

  if (other)
    other->parent = remove->parent;

  if (!remove->parent) {
    tree->root = other;
  } else {
    if (remove == remove->parent->left)
      remove->parent->left = other;
    else
      remove->parent->right = other;
  }

  if (node != remove)
    data_copy(tree, data_get(tree, node), data_get(tree, remove));
  free(node);
  return 0;
}

void btree_destroy(btree *tree) {
  if (tree->root) btree_node_close(tree->root);
  free(tree);
}

// assume data elements are char*
void btree_print(btree *tree, void *where) {
  btree_really_print(tree, tree->root, (FILE*) where, 2);
}

void btree_really_print(btree *tree, btree_node *node, FILE *where, unsigned int spaces) {
  fprintf(where, "%*s%s\n", spaces, " ",
      (node ? *(char**) data_get(tree, node) : "NULL"));
  if (node) {
    btree_really_print(tree, node->left, where, spaces + 4);
    btree_really_print(tree, node->right, where, spaces + 4);
  }
}

static btree_node* btree_node_search(btree* tree, btree_node* node, void *key) {
  int result;
  if (node) {
    while (node && (result = tree->cmp(data_get(tree, node), key))) {
      if (result >0) node = node->left;
      else node = node->right;
    }
  }
  return node;
}

static btree_node* btree_node_minimum(btree_node *node) {
  if (node) {
    while (node->left)
      node = node->left;
  }
  return node;
}

static btree_node* btree_node_maximum(btree_node *node) {
  if (node) {
    while (node->right)
      node = node->right;
  }
  return node;
}

static btree_node* btree_node_successor(btree_node *node) {
  if (node->right) {
    node = btree_node_minimum(node->right);
  } else {
    btree_node *parent = node->parent;
    while (parent && node == parent->right) {
      node = parent;
      parent = node->parent;
    }
    node = parent;
  }
  return node;
}

static btree_node* btree_node_make(btree *tree, void *data) {
  btree_node *ret = (btree_node*) malloc(tree->node_size + tree->elem_size);
  if (ret) {
    data_copy(tree, data_get(tree, ret), data);
    ret->parent = ret->left = ret->right = NULL;
  }
  return ret;
}

static void btree_node_close(btree_node *node) {
  if (node->left)
    btree_node_close(node->left);
  if (node->right)
    btree_node_close(node->right);
  free(node);
}

static int cmp(const void *a, const void *b) {
  int *pa = (int*)a, *pb = (int*)b;
  int ia = *pa, ib = *pb;
  if (ia < ib) return -1;
  else if (ia == ib) return 0;
  else return 1;
}

int main() {
  const static int size = 32;
  btree *tree = btree_create(size, cmp);
  assert(btree_empty(tree));

  int i;
  for (i = 0; i < size; ++i) {
    btree_insert(tree, &i);
  }

  FILE* fp = stdout;
  btree_print(tree, fp);
}
