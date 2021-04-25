HKDF_SRC = hkdf-shaX_512.c \
	../../hkdf.c \
	../wpa_supplicant/src/crypto/sha512-kdf.c \
	../wpa_supplicant/src/crypto/sha512.c \
	../ed25519/sha512.c

SHA3_SRC = ../linux/crypto/sha3_generic.c

PROG = sha3-file hkdf-shaX_512 xor32-file

CFLAGS += -I../../../include/

all: $(PROG)

sha3-file: sha3-file.c $(SHA3_SRC)
	gcc $(CFLAGS) -o $@ $^

xor32-file: xor32-file.c
	gcc $(CFLAGS) -o $@ $^

hkdf-shaX_512: $(HKDF_SRC) $(SHA3_SRC)
	gcc $(CFLAGS) -o $@ $^

clean:
	@-rm -f $(PROG) *.o *.bin
