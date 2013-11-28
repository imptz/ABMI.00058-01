#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "crc16.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Error: nepravilnoe chislo parametrov" << std::endl;
		return -1;
	}

	struct stat fileStat;
	if (stat(argv[1], &fileStat) == -1)
		std::cout << "Error: razmer faila ne opredelen" << std::endl;
	else
	{
		std::cout << "Size: " << fileStat.st_size << std::endl;
		unsigned char* pData = new unsigned char[fileStat.st_size + 8];
		if (pData == nullptr)
			std::cout << "Error: command : memory alloc" << std::endl;
		else
		{
			int fd = open(argv[1], O_RDWR);
			if (fd == -1)
				std::cout << "Error: command : image file open" << std::endl;
			else
			{
				if (read(fd, &pData[4], fileStat.st_size) != fileStat.st_size)
					std::cout << "Error: command : image file read" << std::endl;
				else
				{
					*(reinterpret_cast<unsigned int*>(&pData[0])) = fileStat.st_size;

					unsigned int crc =  Crc16::calcCRC16(&pData[4], fileStat.st_size);
					*(reinterpret_cast<unsigned int*>(&pData[fileStat.st_size + 4])) = crc;
					std::cout << "crc: " << crc << std::endl;

					if (argc == 3)
					{
						unsigned int div = atoi(argv[2]);
						if ((div > 0) && (div < 10000))
						{
							*(reinterpret_cast<unsigned int*>(&pData[0])) /= div;	
							*(reinterpret_cast<unsigned int*>(&pData[0])) += 1;	
						}
					}
					
					lseek(fd, 0, SEEK_SET);

					if (write(fd, pData, fileStat.st_size + 8) != (fileStat.st_size + 8))
						std::cout << "Error: command : write file" << std::endl;
					else
					{
						std::cout << "File zapisan" << std::endl;
					}
				}
		
				close(fd);							
			}

			delete[] pData;
		}
	}

	return 0;
}

