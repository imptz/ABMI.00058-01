#ifndef LOW_DISPLAY_H
#define LOW_DISPLAY_H

extern void print(const char* pStr, int x, int y);
extern void printChar(unsigned char _char, unsigned int x, unsigned int y);
extern char toChar(unsigned char value);
extern char* toString(unsigned int value, char* cStr);
extern void printUInt(unsigned int value, unsigned int x, unsigned int y);

extern int getNextPos();
#endif

