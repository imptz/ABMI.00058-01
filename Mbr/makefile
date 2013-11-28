TARGET_NAME = mbr.bin

all: mbr.s
	nasm -f bin -o $(TARGET_NAME) -w+orphan-labels mbr.s

.PHONY: clean upload

clean:
	rm -f $(TARGET_NAME)

upload:
	sudo dd if=mbr.bin of=/dev/sdb seek=0 bs=512 count=1000000
