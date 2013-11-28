#ifndef LOW_H
#define LOW_H

#define MOV_EAX_CR4 \
    __asm _emit 0x0F \
    __asm _emit 0x20 \
    __asm _emit 0xE0

#define MOV_CR4_EAX \
    __asm _emit 0x0F \
    __asm _emit 0x22 \
    __asm _emit 0xE0

extern "C" int _purecall(void);
extern "C" void _chkstk();

#endif

