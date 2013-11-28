CFLAGS = -Wall -pthread -std=gnu++0x -o3
LIBS =
INCLUDEDIRS = ./Include
SOURCEDIRS = ./Src
LIBDIRS =
OUTPUT = imageCRC 
SEARCH_WILDCARDS := $(addsuffix *.cpp,$(SOURCEDIRS)/)
CC = g++

$(OUTPUT): $(patsubst %.cpp, %.o, $(wildcard  $(SEARCH_WILDCARDS)))
	$(CC) $^ $(CFLAGS) $(addprefix -I,$(INCLUDEDIRS)) $(addprefix -L,$(LIBDIRS)) $(LIBS) -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(addprefix -I,$(INCLUDEDIRS)) $< -o $@

.PHONY: clean run

clean:
	rm -f ./Src/*.o $(OUTPUT)

