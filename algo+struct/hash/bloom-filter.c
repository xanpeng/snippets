//
// http://www.wzxue.com/bloom-filter-%E8%87%AA%E8%A7%A3/
//
// 一种利用多次hash计算和查表判断输入是否属于已知集合的概率性算法。主要用来“判负”(即判定不属于)，因为false negative概率为0。存在0.61m/n概率的false positive(判定属于而其实不属于)。
// bloom filter算法是一种利用hash算法特性的概率性算法，用来判断input是否属于已知集合。
// 1. 实现原理：
// 1.1. 构建：利用k个hash算法计算得到已知hash数组(m bits)的k个index，然后将这些位置的值设置为1。
// 1.2. 查询：利用k个hash函数计算hash数组的k个index，对应值如果全1则返回存在，否则只要有一个0就返回不存在。
// 2. 优点：时间复杂度和空间复杂度远超其他算法。
// 3. 缺点：存在false positive。
// hash算法的特性：hash算法的特性之一是幂等。
// 给定一个字符串集合，先行hash计算并存表，此时判断输入的字符串s是否在集合中，如果s在集合中则必有hash(s).val=1，如果s不在集合中则可能有hash(s).val=1。
// 反过来说，如果hash(s).val=1则s可能在集合中，如果hash(s).val=0则s不可能在集合中。
// 根据hash(s).val=1判定s在集合中而事实s不在集合中，称之为false positive。这里false positive是可能的。
// 根据hash(s).val=0判断s不在集合中而事实s在集合中，称之为false negative。显然这里false negative是不可能的。
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LENGTH 1000000
#define TARGET 10000

typedef unsigned char bit;

bit* init(int length) {
  bit *bits = (bit*) malloc(sizeof(bit)*length);
  int i;
  for (i = 0; i < length; i++)
    bits[i]=0;
  return bits;
}

void add_value(bit* bits, int s) {
  srand(s);
  int i;
  int temp;

  for (i = 0; i < TARGET; i++) {
    temp=rand() % LENGTH;
    bits[temp]=1;
  }
}

int judge(bit* bits,int s) {
  srand(s);
  int i;
  for (i = 0; i < TARGET; i++)
    if (bits[rand()%LENGTH]==0) return 1;
  return 0;
}

int main() {
  bit* bits = init(LENGTH);
  int save[TARGET];
  int i;

  srand((time(NULL)));
  for (i = 0; i < TARGET; i++)
    save[i] = rand();

  for (i = 0; i < TARGET/2; i++)
    add_value(bits, save[i]);

  int num=0;
  for (i = 0; i < TARGET; i++)
    if (judge(bits, save[i])==0) num++;
  printf("\nenter the num is %d",TARGET);
  printf("\n%d",num);
  return 0;
}
