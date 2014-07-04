#include <stdio.h>
#include <setjmp.h>

#define mark printf("[DEBUG] %s:%d\n", __func__, __LINE__)
jmp_buf main_frame;

void functionc(void)
{
    mark;
    longjmp(main_frame, 0);
    mark;
}

void functionb(void)
{
    mark;
    functionc();
    mark;
}

void functiona(void)
{
    mark;
    functionb();
    mark;
}

int main()
{
    mark;
    if (setjmp(main_frame) == 0)
        functiona();
    else
        puts("caught error");

    mark;

    return 0;
}

/*****
[DEBUG] main:30
[DEBUG] functiona:23
[DEBUG] functionb:16
[DEBUG] functionc:9
caught error
[DEBUG] main:36
*****/
