boot.efi: main.o
	lld-21 -flavor link -subsystem:efi_application -entry:efi_main main.o -out:boot.efi

main.o: main.c
	clang-21 -std=c23 -ffreestanding -MMD -mno-red-zone -std=c11 -target i686-unknown-windows -c main.c -o main.o

.PHONY: clean
clean:
	rm -rf *.o *.d *.efi
