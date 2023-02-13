// https://stackoverflow.com/questions/8401689/best-practices-for-recovering-from-a-segmentation-fault

#include <signal.h>
#include <stdio.h>
#include <setjmp.h>

//Declaring global jmp_buf variable to be used by both main and signal handler
static jmp_buf buf;


void magic_handler(int s)
{
    switch(s)
    {

        case SIGSEGV:
        printf("\nSegmentation fault signal caught! Attempting recovery..");
        longjmp(buf, 1);
        break;
    }
    printf("\nAfter switch. Won't be reached");
}



int exceptionTest1(void)
{
    int *p = NULL;
    signal(SIGSEGV, magic_handler);

    if(!setjmp(buf))
    {
         //Trying to dereference a null pointer will cause a segmentation fault, 
         //which is handled by our magic_handler now.
         *p=0xdead;
    }
    else
    {
        printf("\n[1]Successfully recovered! Welcome back in main!!\n"); 
    }
    return 0;
}

int exceptionTest2(void)
{
    int *p = NULL;
    signal(SIGSEGV, magic_handler);

    if(!setjmp(buf))
    {
         //Trying to dereference a null pointer will cause a segmentation fault, 
         //which is handled by our magic_handler now.
         *p=0xdead;
    }
    else
    {
        printf("\n[2]Successfully recovered! Welcome back in main!!\n"); 
    }
    return 0;
}