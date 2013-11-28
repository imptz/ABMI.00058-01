#ifndef CRC16_H
#define CRC16_h

class Crc16{
public:
	static unsigned short calcCRC16(unsigned char *data, int length);
};

#endif
