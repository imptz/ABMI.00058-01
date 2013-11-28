TARGET_NAME = loader.bin

all: loader.s
	nasm -f bin -o $(TARGET_NAME) -w+orphan-labels loader.s

.PHONY: clean upload
 
clean:
	rm -f $(TARGET_NAME)

upload:
	sudo dd if=loader.bin of=/dev/sdb seek=1 bs=512 count=1000000
