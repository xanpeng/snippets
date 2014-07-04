#include <stdio.h>
#include "cexception.h"

#define mark printf("[DEBUG] %s:%d\n", __func__, __LINE__)

void functionc(void)
{
    mark;
    Throw(1);
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
    CEXCEPTION_T e;
    Try {
        mark;
        functiona();
        mark;
    }
    Catch(e) {
        printf("there was an error: %d\n", e);
        return e;
    }

    return 0;
}

/****************
[DEBUG] main:31
[DEBUG] functiona:22
[DEBUG] functionb:15
[DEBUG] functionc:8
there was an error: 1
****************/
