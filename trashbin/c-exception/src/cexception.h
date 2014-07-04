#ifndef _CEXCEPTION_H
#define _CEXCEPTION_H

#include <setjmp.h>

#ifdef __cplusplus
extern "C"
{
#endif

//This is the value to assign when there isn't an exception
#ifndef CEXCEPTION_NONE
#define CEXCEPTION_NONE      (0x5A5A5A5A)
#endif

//This is number of exception stacks to keep track of (one per task)
#ifndef CEXCEPTION_NUM_ID
#define CEXCEPTION_NUM_ID    (1) //there is only the one stack by default
#endif

//This is the method of getting the current exception stack index (0 if only one stack)
#ifndef CEXCEPTION_GET_ID
#define CEXCEPTION_GET_ID    (0) //use the first index always because there is only one anyway
#endif

//The type to use to store the exception values.
#ifndef CEXCEPTION_T
#define CEXCEPTION_T         unsigned int
#endif

//This is an optional special handler for when there is no global Catch
#ifndef CEXCEPTION_NO_CATCH_HANDLER
#define CEXCEPTION_NO_CATCH_HANDLER(id)
#endif

//exception frame structures
typedef struct {
    jmp_buf* pframe;
    CEXCEPTION_T volatile exception;
} CEXCEPTION_FRAME_T;

//actual root frame storage (only one if single-tasking)
extern volatile CEXCEPTION_FRAME_T cexception_frames[];

//Try (see C file for explanation)
#define Try                                                         \
    {                                                               \
        jmp_buf *prev_frame, new_frame;                             \
        unsigned int MY_ID = CEXCEPTION_GET_ID;                     \
        prev_frame = cexception_frames[CEXCEPTION_GET_ID].pframe;   \
        cexception_frames[MY_ID].pframe = (jmp_buf*)(&new_frame);   \
        cexception_frames[MY_ID].exception = CEXCEPTION_NONE;       \
        if (setjmp(new_frame) == 0) {                               \
            if (&prev_frame) 

//Catch (see C file for explanation)
#define Catch(e)                                                    \
            else { }                                                \
            cexception_frames[MY_ID].exception = CEXCEPTION_NONE;   \
        }                                                           \
        else                                                        \
        { e = cexception_frames[MY_ID].exception; e=e; }            \
        cexception_frames[MY_ID].pframe = prev_frame;               \
    }                                                               \
    if (cexception_frames[CEXCEPTION_GET_ID].exception != CEXCEPTION_NONE)

//Throw an Error
void Throw(CEXCEPTION_T exception_id);

#ifdef __cplusplus
}   // extern "C"
#endif


#endif // _CEXCEPTION_H
