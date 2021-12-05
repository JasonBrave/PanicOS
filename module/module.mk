CFLAGS += -I../modlib/ -fPIE -fno-builtin

$(MOD).mod : $(OBJS)
	$(LD) -pie --no-dynamic-linker -e module_init $(OBJS) -o $(MOD).mod

.PHONY: install
install: $(MOD).mod
	cp $(MOD).mod ..

.PHONY: clean
clean:
	rm -f $(MOD).mod $(OBJS)
