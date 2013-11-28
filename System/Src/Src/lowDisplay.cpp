#include "lowDisplay.h"

int nextPos = 0;

int getNextPos(){
	return nextPos++;
}

void print(const char* pStr, int x, int y){
	asm volatile ("pushfl");
	asm volatile ("pushal");

	unsigned int vOffset = 0x000b8000 +  y * 160 + x * 2;
	unsigned int color =  0;
	color = color << 3;
	color += 0;
	color = color << 4;
	color += 15;
	color = color << 8;

	asm volatile ("\tmovl	%0, %%esi\n"::"m"(pStr));
	asm volatile ("\tmovl	%0, %%edi\n"::"m"(vOffset));
	asm volatile ("\tmovl	%0, %%eax\n"::"m"(color));
	asm volatile ("\t1:\n");
	asm volatile ("\tlodsb\n");
	asm volatile ("\tcmpb	$0, %al\n");
	asm volatile ("\tje		2f\n");
	asm volatile ("\tstosw\n");
	asm volatile ("\tjmp	1b\n");
	asm volatile ("\t2:\n");
	asm volatile ("popal");
	asm volatile ("popfl");
}

void printChar(unsigned char _char, unsigned int x, unsigned int y){
	unsigned int vOffset = 0x000b8000 +  y * 160 + x * 2;
	unsigned int _color =  0;
	_color = _color << 3;
	_color += 0;
	_color = _color << 4;
	_color += 15;//CHAR_COLOR;
	_color = _color << 8;
	*reinterpret_cast<unsigned short*>(vOffset) = _color + _char;
}

char toChar(unsigned char value){
	if (value < 10)
		return value + 48;
	else
		return value + 87;
}

char* toString(unsigned int value, char* cStr){
	cStr[0] = toChar(((value & 0xf0000000) >> 28));
	cStr[1] = toChar(((value & 0x0f000000) >> 24));
	cStr[2] = toChar(((value & 0x00f00000) >> 20));
	cStr[3] = toChar(((value & 0x000f0000) >> 16));
	cStr[4] = toChar(((value & 0x0000f000) >> 12));
	cStr[5] = toChar(((value & 0x00000f00) >> 8));
	cStr[6] = toChar(((value & 0x000000f0) >> 4));
	cStr[7] = toChar((value & 0x0000000f));
	cStr[8] = 0;

	return cStr;
}

void printUInt(unsigned int value, unsigned int x, unsigned int y){
	char cStr[12];
	print(toString(value, cStr), x, y);
}


