TARGET_NAME = uso.bin
TARGET_DIR = Bin

SRC_DIR = Src

OBJECTS_DIR = Obj
OBJECTS := $(patsubst $(SRC_DIR)/%, %, $(wildcard $(addsuffix /*.s, $(SRC_DIR)))) $(patsubst $(SRC_DIR)/%, %, $(wildcard $(addsuffix /*.cpp, $(SRC_DIR))))
OBJECTS := $(OBJECTS:.cpp=.o)
OBJECTS := $(OBJECTS:.s=.o)

INCLUDE_DIR = Include

CXX = g++-4.8 -m32
STD_FLAG = -std=gnu++11

OPTIMIZATION =  -O2
CPU_FLAGS = -mtune=geode

CXXFLAGS = -Wall -MD $(STD_FLAG) -ffreestanding -fno-asynchronous-unwind-tables -fno-rtti -fno-exceptions -fpermissive $(OPTIMIZATION) $(CPU_FLAGS)
CXXFLAGS += -nostdinc++ -nostdinc -fno-threadsafe-statics 
LDFLAGS = -Tlinker.ld -e_start -m elf_i386 -Ttext=0x00100000 -Map mapfile -nostdlib -nodefaultlibs

VPATH = $(SRC_DIR)

all: $(OBJECTS_DIR) $(TARGET_DIR) $(OBJECTS)
	ld $(LDFLAGS) -s  -o $(TARGET_DIR)/$(TARGET_NAME) $(addprefix $(OBJECTS_DIR)/, $(OBJECTS))
	../../ImageCRC/imageCRC $(TARGET_DIR)/$(TARGET_NAME)

noCrc: $(OBJECTS_DIR) $(TARGET_DIR) $(OBJECTS)
	ld $(LDFLAGS) -o $(TARGET_DIR)/$(TARGET_NAME) $(addprefix $(OBJECTS_DIR)/, $(OBJECTS))

%.o: %.s
	nasm -felf32 -o $(OBJECTS_DIR)/$@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $(OBJECTS_DIR)/$@ $^

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(OBJECTS_DIR):
	mkdir -p $(OBJECTS_DIR)

.PHONY: clean upload

clean:
	rm -rf $(TARGET_DIR) $(OBJECTS_DIR)
	rm -f mapfile

upload:
#	sudo ../../ImageLoader/imageLoader /dev/ttyUSB0 run $(TARGET_DIR)/$(TARGET_NAME)
	sudo ../../ImageLoader/imageLoader /dev/ttyS0 run $(TARGET_DIR)/$(TARGET_NAME)

include $(wildcard $(addsuffix /*.d, $(OBJECTS_DIR)))
