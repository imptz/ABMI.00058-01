#ifndef CRC16_H
#define CRC16_H

class Crc16
{
	public:
		static unsigned short calcCRC16(unsigned char *data, int length);
};

#endif
