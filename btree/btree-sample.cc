// from http://blog.csdn.net/v_july_v/article/details/6735293
#include <stdlib.h>  
#include <stdio.h>  
#include <assert.h>

#define cmp(a, b) ((((a)-(b)) >= 0) ? 1 : 0)
#define BTREE_D 2  
#define ORDER (BTREE_D * 2) //定义为4阶B-tree,2-3-4树。最简单为3阶B-tree,2-3树。

typedef int key_type;  
typedef struct btree_node {  
  int keynum;                        // 结点中关键字的个数，keynum <= btree_N  
  key_type key[ORDER-1];             // 关键字向量为key[0..keynum - 1]  
  struct btree_node* child[ORDER];   // 孩子指针向量为child[0..keynum]  
  bool is_leaf;                       // 是否是叶子节点的标志  
} btree_node;  
typedef btree_node* btree;

void btree_create(btree* tree, const key_type* data, int length);  
void btree_destroy(btree* tree);  
void btree_insert(btree* tree, key_type key);  
void btree_remove(btree* tree, key_type key);  
void btree_print(const btree tree, int layer=1);  
btree_node* btree_search(const btree tree, int key, int* pos); 

void disk_write(btree_node* node) {  
  printf("向磁盘写入节点");  
  for(int i = 0; i < ORDER - 1; i++)
    printf("%c", node->key[i]);  
  printf("\n");  
}  

void disk_read(btree_node** node)  {  
  printf("从磁盘读取节点");  
  for(int i = 0; i < ORDER - 1; i++)
    printf("%c", (*node)->key[i]);  
  printf("\n");  
}  

void btree_print(const btree tree, int layer) {  
  int i;  
  btree_node* node = tree;  
  if (node) {  
    printf("第%d层，%d node: ", layer, node->keynum);  
    for (i = 0; i < ORDER-1; ++i)
      printf("%c ", node->key[i]);  
    printf("\n"); 

    ++layer;  
    for (i = 0 ; i <= node->keynum; i++)
      if (node->child[i]) btree_print(node->child[i], layer);  
  } else {
    printf("树为空。\n");  
  }  
}  

/*************************************************************************************** 
  将分裂的结点中的一半元素给新建的结点，并且将分裂结点中的中间关键字元素上移至父节点中。 
  parent 是一个非满的父节点 
  node 是 tree 孩子表中下标为 index 的孩子节点，且是满的，需分裂。 
 *******************************************************************/  
void btree_split_child(btree_node* parent, int index, btree_node* node) {  
  printf("btree_split_child!\n");  
  assert(parent && node);  
  int i;  

  // 创建新节点，存储 node 中后半部分的数据  
  btree_node* newnode = (btree_node*)calloc(sizeof(btree_node), 1);  
  if (!newnode) {  
    printf("Error! out of memory!\n");  
    return;  
  }  
  newnode->is_leaf = node->is_leaf;  
  newnode->keynum = BTREE_D - 1;  

  // 拷贝 node 后半部分关键字,然后将node后半部分置为0。  
  for (i = 0; i < newnode->keynum; ++i){  
    newnode->key[i] = node->key[BTREE_D + i];  
    node->key[BTREE_D + i] = 0;  
  }  

  // 如果 node 不是叶子节点，拷贝 node 后半部分的指向孩子节点的指针，然后将node后半部分指向孩子节点的指针置为NULL。  
  if (!node->is_leaf) {  
    for (i = 0; i < BTREE_D; i++) {  
      newnode->child[i] = node->child[BTREE_D + i];  
      node->child[BTREE_D + i] = NULL;  
    }  
  }  

  // 将 node 分裂出 newNode 之后，里面的数据减半  
  node->keynum = BTREE_D - 1;  

  // 调整父节点中的指向孩子的指针和关键字元素。分裂时父节点增加指向孩子的指针和关键元素。  
  for (i = parent->keynum; i > index; --i)
    parent->child[i + 1] = parent->child[i];  
  parent->child[index + 1] = newnode;  

  for (i = parent->keynum - 1; i >= index; --i)
    parent->key[i + 1] = parent->key[i];  
  parent->key[index] = node->key[BTREE_D - 1];  
  ++parent->keynum;  

  node->key[BTREE_D - 1] = 0;  

  // 写入磁盘  
  disk_write(parent);  
  disk_write(newnode);  
  disk_write(node);  
}  

void btree_insert_nonfull(btree_node* node, key_type key) {  
  assert(node);  
  int i;  
  if (node->is_leaf) { // 节点是叶子节点，直接插入  
    i = node->keynum - 1;  
    while (i >= 0 && key < node->key[i]) {  
      node->key[i + 1] = node->key[i];  
      --i;  
    }  
    node->key[i + 1] = key;  
    ++node->keynum;  
    disk_write(node);  // 写入磁盘  
  } else { // 节点是内部节点  
    // 查找插入的位置  
    i = node->keynum - 1;  
    while (i >= 0 && key < node->key[i]) {  
      --i;  
    }  
    ++i;  
    disk_read(&node->child[i]); // 从磁盘读取孩子节点  
    // 如果该孩子节点已满，分裂调整值  
    if (node->child[i]->keynum == (ORDER-1)) {  
      btree_split_child(node, i, node->child[i]);  
      // 如果待插入的关键字大于该分裂结点中上移到父节点的关键字，在该关键字的右孩子结点中进行插入操作。  
      if (key > node->key[i]) ++i;  
    }  
    btree_insert_nonfull(node->child[i], key);  
  }  
}  

void btree_insert(btree* tree, key_type key) {  
  printf("btree_insert:\n");  
  btree_node* node;  
  btree_node* root = *tree;  

  if (NULL == root) {  
    root = (btree_node*)calloc(sizeof(btree_node), 1);  
    if (!root) {  
      printf("Error! out of memory!\n");  
      return;  
    }  
    root->is_leaf = true;  
    root->keynum = 1;  
    root->key[0] = key;  
    *tree = root;  
    disk_write(root);  // 写入磁盘  
    return;  
  }  

  if (root->keynum == (ORDER-1)) {  // 根节点已满，插入前需要进行分裂调整  
    node = (btree_node*)calloc(sizeof(btree_node), 1);  // 产生新节点当作根  
    if (!node) {  
      printf("Error! out of memory!\n");  
      return;  
    }  
    *tree = node;  
    node->is_leaf = false;  
    node->keynum = 0;  
    node->child[0] = root;  

    btree_split_child(node, 0, root);  
    btree_insert_nonfull(node, key);  
  } else { // 根节点未满，在当前节点中插入 key  
    btree_insert_nonfull(root, key);  
  }  
}  

// 对 tree 中的节点 node 进行合并孩子节点处理.  
// 注意：孩子节点的 keynum 必须均已达到下限，即均等于 BTREE_D - 1  
// 将 tree 中索引为 index 的 key 下移至左孩子结点中，  
// 将 node 中索引为 index + 1 的孩子节点合并到索引为 index 的孩子节点中，右孩子合并到左孩子结点中。  
// 并调相关的 key 和指针。
void btree_merge_child(btree* tree, btree_node* node, int index) {  
  printf("btree_merge_child!\n");  
  assert(tree && node && index >= 0 && index < node->keynum);  

  int i;  
  key_type key = node->key[index];  
  btree_node* left_child = node->child[index];  
  btree_node* right_child = node->child[index + 1];  

  assert(left_child && left_child->keynum == BTREE_D - 1  
      && right_child && right_child->keynum == BTREE_D - 1);  

  // 将 node中关键字下标为index的key下移至左孩子结点中，该key所对应的右孩子结点指向node的右孩子结点中的第一个孩子。  
  left_child->key[left_child->keynum] = key;  
  left_child->child[left_child->keynum + 1] = right_child->child[0];  
  ++left_child->keynum;  

  // 右孩子的元素合并到左孩子结点中。  
  for (i = 0; i < right_child->keynum; ++i) {  
    left_child->key[left_child->keynum] = right_child->key[i];  
    left_child->child[left_child->keynum + 1] = right_child->child[i + 1];  
    ++left_child->keynum;  
  }  

  // 在 node 中下移的 key后面的元素前移  
  for (i = index; i < node->keynum - 1; ++i) {  
    node->key[i] = node->key[i + 1];  
    node->child[i + 1] = node->child[i + 2];  
  }  
  node->key[node->keynum - 1] = 0;  
  node->child[node->keynum] = NULL;  
  --node->keynum;  

  // 如果根节点没有 key 了，并将根节点调整为合并后的左孩子节点；然后删除释放空间。  
  if (node->keynum == 0) {  
    if (*tree == node) *tree = left_child;  
    free(node);  
    node = NULL;  
  }  
  free(right_child);  
  right_child = NULL;  
}  

void btree_recursive_remove(btree* tree, key_type key) {  
  // B-数的保持条件之一：非根节点的内部节点的关键字数目不能少于BTREE_D - 1  
  int i, j, index;  
  btree_node *root = *tree;  
  btree_node *node = root;  
  if (!root) {  
    printf("Failed to remove %c, it is not in the tree!\n", key);  
    return;  
  }  

  // 结点中找key。  
  index = 0;  
  while (index < node->keynum && key > node->key[index]) {  
    ++index;  
  }  

/*======================含有key的当前结点时的情况==================== 
node: 
index of Key:            i-1  i  i+1 
                        +---+---+---+---+ 
                          *  key   * 
                    +---+---+---+---+---+ 
                           /     \ 
index of Child:           i      i+1 
                         /         \ 
                    +---+---+      +---+---+ 
                      *   *           *   *    
                +---+---+---+  +---+---+---+ 
                    leftChild     rightChild 
============================================================*/ 
  /*一、结点中找到了关键字key的情况.*/  
  btree_node *left_child, *right_child;  
  key_type left_key, right_key;  
  if (index < node->keynum && node->key[index] == key) {
    /* 1，所在节点是叶子节点，直接删除*/  
    if (node->is_leaf) {  
      for (i = index; i < node->keynum-1; ++i) {  
        node->key[i] = node->key[i + 1];  
        //node->child[i + 1] = node->child[i + 2];叶子节点的孩子结点为空，无需移动处理。  
      }  
      node->key[node->keynum-1] = 0;  
      --node->keynum;  

      if (node->keynum == 0) {  
        assert(node == *tree);  
        free(node);  
        *tree = NULL;  
      }  
      return;  
    }  
    /*2.选择脱贫致富的孩子结点。*/  
    // 2a，选择相对富有的左孩子结点。  
    // 如果位于 key 前的左孩子结点的 key 数目 >= BTREE_D，  
    // 在其中找 key 的左孩子结点的最后一个元素上移至父节点key的位置。  
    // 然后在左孩子节点中递归删除元素leftKey。  
    else if (node->child[index]->keynum >= BTREE_D) {  
      left_child = node->child[index];  
      left_key = left_child->key[left_child->keynum - 1];  
      node->key[index] = left_key;  
      btree_recursive_remove(&left_child, left_key);  
    }  
    // 2b，选择相对富有的右孩子结点。  
    // 如果位于 key 后的右孩子结点的 key 数目 >= BTREE_D，  
    // 在其中找 key 的右孩子结点的第一个元素上移至父节点key的位置  
    // 然后在右孩子节点中递归删除元素rightKey。  
    else if (node->child[index + 1]->keynum >= BTREE_D) {  
      right_child = node->child[index + 1];  
      right_key = right_child->key[0];  
      node->key[index] = right_key;  
      btree_recursive_remove(&right_child, right_key);  
    }  
    /*左右孩子结点都刚脱贫。删除前需要孩子结点的合并操作*/  
    // 2c，左右孩子结点只包含 BTREE_D - 1 个节点，  
    // 合并是将 key 下移至左孩子节点，并将右孩子节点合并到左孩子节点中，  
    // 删除右孩子节点，在父节点node中移除 key 和指向右孩子节点的指针，  
    // 然后在合并了的左孩子节点中递归删除元素key。  
    else if (node->child[index]->keynum == BTREE_D - 1  
        && node->child[index + 1]->keynum == BTREE_D - 1){  
      left_child = node->child[index];  
      btree_merge_child(tree, node, index);  
      btree_recursive_remove(&left_child, key);  // 在合并了的左孩子节点中递归删除 key  
    }  
  }  

/*======================未含有key的当前结点时的情况==================== 
node: 
index of Key:            i-1  i  i+1 
                        +---+---+---+---+ 
                          *  keyi * 
                    +---+---+---+---+---+ 
                       /    |    \ 
index of Child:      i-1    i     i+1 
                     /      |       \ 
            +---+---+   +---+---+      +---+---+ 
             *   *        *   *          *   *    
        +---+---+---+   +---+---+---+  +---+---+---+ 
        left_sibling       Child        right_sibling  
============================================================*/ 
  /*二、结点中未找到了关键字key的情况.*/  
  else {  
    btree_node *left_sibling, *right_sibling, *child;  
    // 3. key 不在内节点 node 中，则应当在某个包含 key 的子节点中。  
    //  key < node->key[index], 所以 key 应当在孩子节点 node->child[index] 中  
    child = node->child[index];  
    if (!child) {  
      printf("Failed to remove %c, it is not in the tree!\n", key);  
      return;  
    }  
    /*所需查找的该孩子结点刚脱贫的情况*/  
    if (child->keynum == BTREE_D - 1) {  
      left_sibling = NULL;  
      right_sibling = NULL;  
      if (index - 1 >= 0) left_sibling = node->child[index - 1];  
      if (index + 1 <= node->keynum) right_sibling = node->child[index + 1];  

      /*选择致富的相邻兄弟结点。*/  
      // 3a，如果所在孩子节点相邻的兄弟节点中有节点至少包含 BTREE_D 个关键字  
      // 将 node 的一个关键字key[index]下移到 child 中，将相对富有的相邻兄弟节点中一个关键字上移到  
      // node 中，然后在 child 孩子节点中递归删除 key。  
      if ((left_sibling && left_sibling->keynum >= BTREE_D) || (right_sibling && right_sibling->keynum >= BTREE_D)) {  
        int rich_right = 0;  
        if(right_sibling) rich_right = 1;  
        if(left_sibling && right_sibling) rich_right = cmp(right_sibling->keynum, left_sibling->keynum);  
        if (right_sibling && right_sibling->keynum >= BTREE_D && rich_right) {  
          //相邻右兄弟相对富有，则该孩子先向父节点借一个元素，右兄弟中的第一个元素上移至父节点所借位置，并进行相应调整。  
          child->key[child->keynum] = node->key[index];  
          child->child[child->keynum + 1] = right_sibling->child[0];  
          ++child->keynum;  

          node->key[index] = right_sibling->key[0];  
          for (j = 0; j < right_sibling->keynum - 1; ++j) { //元素前移  
            right_sibling->key[j] = right_sibling->key[j + 1];  
            right_sibling->child[j] = right_sibling->child[j + 1];  
          }  
          right_sibling->key[right_sibling->keynum-1] = 0;  
          right_sibling->child[right_sibling->keynum-1] = right_sibling->child[right_sibling->keynum];  
          right_sibling->child[right_sibling->keynum] = NULL;  
          --right_sibling->keynum;  
        }  
        else { //相邻左兄弟相对富有，则该孩子向父节点借一个元素，左兄弟中的最后元素上移至父节点所借位置，并进行相应调整。  
          for (j = child->keynum; j > 0; --j) {//元素后移  
            child->key[j] = child->key[j - 1];  
            child->child[j + 1] = child->child[j];  
          }  
          child->child[1] = child->child[0];  
          child->child[0] = left_sibling->child[left_sibling->keynum];  
          child->key[0] = node->key[index - 1];  
          ++child->keynum;  

          node->key[index - 1] = left_sibling->key[left_sibling->keynum - 1];  

          left_sibling->key[left_sibling->keynum - 1] = 0;  
          left_sibling->child[left_sibling->keynum] = NULL;  

          --left_sibling->keynum;  
        }  
      }  
      /*相邻兄弟结点都刚脱贫。删除前需要兄弟结点的合并操作,*/  
      // 3b, 如果所在孩子节点相邻的兄弟节点都只包含 BTREE_D - 1 个关键字，  
      // 将 child 与其一相邻节点合并，并将 node 中的一个关键字下降到合并节点中，  
      // 再在 node 中删除那个关键字和相关指针，若 node 的 key 为空，删之，并调整根为合并结点。  
      // 最后，在相关孩子节点child中递归删除 key。  
      else if ((!left_sibling || (left_sibling && left_sibling->keynum == BTREE_D - 1))  
          && (!right_sibling || (right_sibling && right_sibling->keynum == BTREE_D - 1))) {  
        if (left_sibling && left_sibling->keynum == BTREE_D - 1) {  
          btree_merge_child(tree, node, index - 1); //node中的右孩子元素合并到左孩子中。  
          child = left_sibling;  
        } else if (right_sibling && right_sibling->keynum == BTREE_D - 1) {  
          btree_merge_child(tree, node, index);//node中的右孩子元素合并到左孩子中。  
        }  
      }  
    }  
    btree_recursive_remove(&child, key);//调整后，在key所在孩子结点中继续递归删除key。  
  }  
}  

void btree_remove(btree* tree, key_type key) {  
  printf("btree_remove:\n");  
  if (*tree==NULL) {     
    printf("btree is NULL!\n");  
    return;  
  }  
  btree_recursive_remove(tree, key);  
}  

btree_node* btree_recursive_search(const btree tree, key_type key, int* pos) {  
  int i = 0;  
  while (i < tree->keynum && key > tree->key[i]) {  
    ++i;  
  }  

  if (i < tree->keynum && tree->key[i] == key) {  
    *pos = i;  
    return tree;  
  }  

  if (tree->is_leaf) return NULL;  // tree为叶子节点，找不到key，查找失败返回  

  // 节点内查找失败，但tree->key[i-1]< key < tree->key[i]，下一个查找的结点应为child[i]  
  // 从磁盘读取第 i 个孩子的数据  
  disk_read(&tree->child[i]);  
  // 递归地继续查找于树 tree->child[i]  
  return btree_recursive_search(tree->child[i], key, pos);  
}  

btree_node* btree_search(const btree tree, key_type key, int* pos) {  
  printf("btree_search:\n");  
  if (!tree) {  
    printf("btree is NULL!\n");  
    return NULL;  
  }  
  *pos = -1;  
  return btree_recursive_search(tree, key, pos);  
}  

void btree_create(btree* tree, const key_type* data, int length) {  
  assert(tree);  
  int i;  
  printf("\n 开始创建 B-树，关键字为:\n");  
  for (i = 0; i < length; i++)
    printf(" %c ", data[i]);  
  printf("\n");  

  for (i = 0; i < length; i++) {  
    printf("\n插入关键字 %c:\n", data[i]);  
    int pos = -1;  
    btree_search(*tree, data[i], &pos);
    if (pos != -1)  
      printf("this key %c is in the B-tree, not to insert.\n", data[i]);  
    else  
      btree_insert(tree, data[i]);

    btree_print(*tree);
  }  
  printf("\n");  
}  

void btree_destroy(btree* tree) {  
  int i;  
  btree_node* node = *tree;  
  if (node) {  
    for (i = 0; i <= node->keynum; i++)
      btree_destroy(&node->child[i]);  
    free(node);  
  }  
  *tree = NULL;  
}  

void test_btree_search(btree tree, key_type key) {  
  int pos = -1;  
  btree_node* node = btree_search(tree, key, &pos);  
  if (node) {  
    printf("在%s节点（包含 %d 个关键字）中找到关键字 %c，其索引为 %d\n",  
        node->is_leaf ? "叶子" : "非叶子",  
        node->keynum, key, pos);  
  } else {  
    printf("在树中找不到关键字 %c\n", key);  
  }  
}  

void test_btree_remove(btree* tree, key_type key) {  
  printf("\n移除关键字 %c \n", key);  
  btree_remove(tree, key);  
  btree_print(*tree);  
  printf("\n");  
}  

void test_btree() {  
  key_type array[] = {  
    'G','G', 'M', 'P', 'X', 'A', 'C', 'D', 'E', 'J', 'K',  
    'N', 'O', 'R', 'S', 'T', 'U', 'V', 'Y', 'Z', 'F', 'X'  
  };  
  const int length = sizeof(array)/sizeof(key_type);  
  btree tree = NULL;  
  key_type key1 = 'R';        // in the tree.  
  key_type key2 = 'B';        // not in the tree.  

  // 创建  
  btree_create(&tree, array, length);  
  printf("\n=== 创建 B- 树 ===\n");  
  btree_print(tree);  
  printf("\n");  

  // 查找  
  test_btree_search(tree, key1);  
  printf("\n");  
  test_btree_search(tree, key2);  

  // 移除不在B树中的元素  
  test_btree_remove(&tree, key2);  
  printf("\n");  

  // 插入关键字  
  printf("\n插入关键字 %c \n", key2);  
  btree_insert(&tree, key2);  
  btree_print(tree);  
  printf("\n");  

  test_btree_search(tree, key2);  

  // 移除关键字  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'M';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'E';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'G';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'A';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'D';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'K';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'P';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'J';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'C';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'X';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'O';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'V';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'R';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'U';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'T';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  key2 = 'N';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  
  key2 = 'S';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  
  key2 = 'Y';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  
  key2 = 'F';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  
  key2 = 'Z';  
  test_btree_remove(&tree, key2);  
  test_btree_search(tree, key2);  

  // 销毁  
  btree_destroy(&tree);  
}  

int main() {  
  test_btree();  
  return 0;  
}  
