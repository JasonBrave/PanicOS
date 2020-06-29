CFLAGS += -I../../library/libsys/include -I../../library/libc/include \
	-I../../library

$(APP): $(OBJS)
	$(LD) -L../../library -static -e _start -Ttext-segment=0 \
	-o $(APP) $(OBJS) ../../library/crt/crt0.o -lc -lsys

%.o : %.asm
	nasm -felf32 $< -o $@

.PHONY: install
install: $(APP)
	cp $(APP) ../../rootfs/

.PHONY: clean
clean:
	rm -f $(APP) $(OBJS)
