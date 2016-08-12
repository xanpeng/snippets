#include<stdio.h>
#include "aa.h"
#include "pp.h"
#include "qq.h"
struct A a;
void init_pp(void)
{
    set_pp(&a);
}
void init_qq(void)
{
    set_qq(&a);
}
 
void print_pp(void)
{
    a.pp();
}
void print_qq(void)
{
    a.qq();
}
