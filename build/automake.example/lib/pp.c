#include <stdio.h>
#include "aa.h"
#include "qq.h"
void pp(void)
{
    printf("pp\n");
}
void set_pp(struct A* aa)
{
    aa->pp=pp;
}
