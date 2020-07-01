bootblock: bootasm.S bootmain.c
	$(CC) $(CFLAGS) -I../kernel -fno-pic -O -I. -c bootmain.c
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c bootasm.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o bootblock.o bootasm.o bootmain.o
	$(OBJCOPY) -S -O binary -j .text bootblock.o bootblock
	../tools/sign.pl bootblock

.PHONY: clean
clean:
	rm -f *.o bootblock
