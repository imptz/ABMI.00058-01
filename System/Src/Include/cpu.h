#ifndef CPU_H
#define CPU_H

class Cpu
{
public:
    static inline void sti(){
        asm volatile ("sti");
    }

    static inline void cli(){
        asm volatile ("cli");
    }

    static inline void reboot(){
        asm volatile ("		cli\n\t"
            "lWait:	inb		$0x64, %al\n\t"
            "		testb	$2, %al\n\t"
            "		jnz		lWait\n\t"
            "		movb	$0xfe, %al\n\t"
            "		outb	%al, $0x64\n\t"
            "lInf:\n\t"
            "		jmp lInf\n\t" );
    }

    static inline void shutdown(){
        asm volatile ("		cli\n\t"
            "		hlt\n\t"
            "sht_lInf:\n\t"
            "		jmp	sht_lInf\n\t");
    }

    static inline void setIdt(unsigned int idtDataAddress){
        asm volatile("lidt (%0)" : : "d" (idtDataAddress));
    }

    static inline void getIdt(unsigned int idtDataAddress){
        asm volatile("sidt (%0)" : : "d" (idtDataAddress));
    }

    static unsigned char inPort(unsigned short port){
        unsigned char res = 0;
        asm volatile ("inb %%dx,%%al;":"=a"(res):"d"(port));

        return res;
    }

    static inline void outPort(unsigned short port, unsigned char value){
        asm volatile ("outb %%al,%%dx;"::"d"(port),"a"(value));
    }

    static inline void outPort(unsigned short port, unsigned int value){
        asm volatile ("outl %%eax,%%dx;"::"d"(port),"a"(value));
    }
};

#endif
