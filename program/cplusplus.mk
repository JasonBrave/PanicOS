CFLAGS += -I../../library/libsys/include -I../../library/libc/include \
	-I../../library/libposix/include -I../../library
CXXFLAGS += -I../../library/libcpp/include -I../../library/libposix/include -I../../library/libsys/include -I../../library

$(APP): $(OBJS)
	$(LD) -L../../library -rpath-link=../../library -I/lib/ld.so -e _start \
	-u _dl_fini -Ttext-segment=0 -o $(APP) $(OBJS) ../../library/crt/crt1.o \
	$(LIB) -lcpp -lposix -lc -lsys $(shell $(CC) -print-libgcc-file-name)

%.o : %.asm
	nasm -felf32 $< -o $@

.PHONY: install
install: $(APP)
	cp $(APP) ../../rootfs/bin/

.PHONY: clean
clean:
	rm -f $(APP) $(OBJS)
