CFLAGS += -I../../lib/include

$(APP): $(OBJS)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $(APP) $(OBJS) ../../lib/lib.a

%.o : %.asm
	nasm -felf32 $< -o $@

.PHONY: install
install: $(APP)
	cp $(APP) ../../rootfs/

.PHONY: clean
clean:
	rm -f $(APP) $(OBJS)
