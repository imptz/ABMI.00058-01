#include "lowDisplay.h"

extern "C" void __cxa_pure_virtual(void)
{
}

extern "C" unsigned long start_ctors, end_ctors;
extern "C" void main(void);

void init(void)
{
    for(unsigned long * constructor(& start_ctors); constructor < & end_ctors; ++constructor)
    	((void (*) (void)) (*constructor)) ();
}
