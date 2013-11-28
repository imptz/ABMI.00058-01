#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "crc16.h"

namespace Fr
{
	int serialPortStart(std::string name, int baudrate)
	{
		int fd;

		fd = open(name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
		if (fd != -1)
			fcntl(fd, F_SETFL, 0);

		termios options;

		memset(&options, 0, sizeof(options));
		options.c_cflag = baudrate  | CS8 | CLOCAL | CREAD;
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;

		options.c_lflag = 0;

		options.c_cc[VTIME] = 1;
		options.c_cc[VMIN] = 5;

		tcflush(fd, TCIFLUSH);
		tcsetattr(fd,TCSANOW,&options);

		return (fd);
	}

	bool connect(int fd)
	{
		unsigned char code[4] = {0x12, 0x34, 0x56, 0x78};
		if (write(fd, code, 4) < 0)
			return false;

		unsigned int pos = 0;
		unsigned int buffer = 0;
		unsigned char sym;

		while (pos < 4)
		{
			if (read(fd, &sym, 1) != 0)
			{
				buffer += (sym <<  (8 * pos));
				pos++;
			}
		}

		if (buffer == 0x87654321)
			return true;
		else
			return false;
	}

	enum COMMAND
	{
		COMMAND_RUN = 2,
		COMMAND_WRITE = 3
	};

	void command(COMMAND code, char* imageFileName, int serialPortFd)
	{
		struct stat fileStat;
		if (stat(imageFileName, &fileStat) == -1)
			std::cout << "Error: razmer faila ne opredelen" << std::endl;
		else
		{
			std::cout << "Size: " << fileStat.st_size << std::endl;
			unsigned char* frame = new unsigned char[fileStat.st_size + 7];
			if (frame == nullptr)
				std::cout << "Error: command : memory alloc" << std::endl;
			else
			{
				int imageFd = open(imageFileName, O_RDONLY);
				if (imageFd == -1)
					std::cout << "Error: command : image file open" << std::endl;
				else
				{
					if (read(imageFd, &frame[5], fileStat.st_size) != fileStat.st_size)
						std::cout << "Error: command : image file read" << std::endl;
					else
					{
						frame[0] = static_cast<unsigned char>(code);

						*(reinterpret_cast<unsigned int*>(&frame[1])) = fileStat.st_size;
						*(reinterpret_cast<unsigned short*>(&frame[fileStat.st_size + 5])) = Crc16::calcCRC16(frame, fileStat.st_size + 5);
				
						if (write(serialPortFd, frame, fileStat.st_size + 7) < 0)
							std::cout << "Error: command : send frame" << std::endl;
						else
						{
							unsigned int pos = 0;
							unsigned int buffer = 0;
							unsigned char sym;

							while (pos < 4)
							{
								if (read(serialPortFd, &sym, 1) != 0)
								{
									buffer += (sym <<  (8 * pos));
									pos++;
								}
							}

							if (buffer == 0x87654321)
								std::cout << "Comanda vipolnena" << std::endl;	
							else
								std::cout << "Error: command : ustroystvo vozvratilo ne pravilniy otvet" << std::endl;
							close(imageFd);							
						}
					}
				}
			}

			delete[] frame;
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cout << "Error: nepravilnoe chislo parametrov" << std::endl;
		return -1;
	}

	int fd = Fr::serialPortStart(argv[1], B115200);
	if (fd < 0)
		return -1;

	if (Fr::connect(fd))
	{
		std::cout << "connection: TRUE" << std::endl;
		if (strcmp(argv[2], "run") == 0)
		{
			std::cout << "COMMAND_RUN: " << argv[3] << std::endl;
			Fr::command(Fr::COMMAND_RUN, argv[3], fd);
		}
		else
			if (strcmp(argv[2], "write") == 0)
			{
				std::cout << "COMMAND_WRITE: " << argv[3] << std::endl;
				Fr::command(Fr::COMMAND_WRITE, argv[3], fd);	
			}
			else	
				std::cout << "ERROR: COMMAND_UNDEFINED" << std::endl;
	}
	else
	{
		std::cout << "connection: FALSE" << std::endl;
	}

	close(fd);

	return 0;
}

