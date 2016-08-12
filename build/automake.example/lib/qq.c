#include <stdio.h>
#include "aa.h"
#include "qq.h"
void qq(void)
{
    printf("qq\n");
}
void set_qq(struct A* aa)
{
    aa->qq=qq;
}
