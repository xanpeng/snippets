//
// http://www.wzxue.com/bloom-filter-%E8%87%AA%E8%A7%A3/
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
