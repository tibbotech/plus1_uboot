rm -f out*.bin

make hkdf-sha512

dd if=/dev/zero of=in.bin bs=1 count=256 2>/dev/null
./hkdf-sha512 32 out.bin in.bin
hexdump -C out.bin

dd if=/dev/urandom of=in.bin bs=1 count=256 2>/dev/null
./hkdf-sha512 32 out2.bin in.bin
hexdump -C out2.bin
