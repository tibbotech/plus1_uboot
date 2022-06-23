.PHONY: default clean distclean

# select sha2 / sha3 by .config and auto_config.h
include ../../../../.config
CFLAGS += -I../../../../include/


BINS := mkey_ed_shaX sign_ed_shaX \
	mkey_x25519 x25519_ss


SRC = burn.c  burnstack.c  ed25519-sha512.c  ed.c  fld.c  sc.c  x25519.c
ifeq ($(CONFIG_USE_SHA2), y)
SRC += sha512.c
endif

SRC := $(foreach source_file, $(SRC),$(filter %.c, ../lib/$(source_file)))

CFLAGS += -I../lib


ifneq ($(CONFIG_USE_SHA2), y)
SRC += ../../linux/crypto/sha3_generic.c
endif

all: $(BINS)
ifeq ($(CONFIG_USE_SHA2), y)
	@echo "Built ed25519-sha2 tools:"
else
	@echo "Built ed25519-sha3 tools:"
endif
	@echo "$(BINS)"

mkey_ed_shaX: mkey_ed25519.c $(SRC)
	gcc $(CFLAGS) -o $@ $^
	strip $@

mkey_x25519: mkey_x25519.c $(SRC)
	gcc $(CFLAGS) -o $@ $^
	strip $@

gen: mkey_ed_shaX mkey_x25519
	./mkey_ed_shaX 1
	./mkey_x25519 1


sign_ed_shaX: sign.c $(SRC)
	gcc $(CFLAGS) -o $@ $^
	strip $@

x25519_ss: x25519_ss.c $(SRC)
	gcc $(CFLAGS) -o $@ $^
	strip $@

test:
	@echo "Gen keys..."
	@make gen >/dev/null
	@echo "Test ed25519 sign"
	@./test_sign.sh >/dev/null
	@echo "Test x25519"
	@./test_x25519.sh >/dev/null
	@echo "PASS"

clean:
	rm -rf *.o $(BINS) keys/ *.sig *.bin
