rm -f in.bin out-s*.bin

make sha3-file

TEST_SIZE=127
echo "Gen zero file, size=$TEST_SIZE"
dd if=/dev/zero of=in.bin bs=1 count=$TEST_SIZE >/dev/null

echo "test SHA3-256"
./sha3-file 256 out-s256.bin in.bin
hexdump -C out-s256.bin

echo "should be:"
openssl dgst -sha3-256 in.bin

echo ""

echo "test SHA3-512"

./sha3-file 512 out-s512.bin in.bin
hexdump -C out-s512.bin
echo "should be:"
openssl dgst -sha3-512 in.bin
