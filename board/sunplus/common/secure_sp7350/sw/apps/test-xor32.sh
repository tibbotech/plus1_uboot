rm -f in.bin out.bin

make xor32-file

TEST_SIZE=12
echo "Gen random file, size=$TEST_SIZE"
dd if=/dev/urandom of=in.bin bs=1 count=$TEST_SIZE >/dev/null

echo "Input:"
lst=`hexdump -v -e '1/4 "0x%08x "' in.bin`
echo $lst

echo "test xor32"
./xor32-file out.bin in.bin 0

echo "Output:"
outv=0x`hexdump -v -e '1/4 "%08x\n"' out.bin`
echo "$outv"
echo ""

# verify XOR32(lst) == outv
xval=0
for val in $lst ;
do
	xval=$((xval ^ val))
done

xval=0x`printf %x $xval`
echo "Shell math xval:"
echo "$xval"

if [ "$xval" != "$outv" ];then
	echo "test fail"
else
	echo "test OK"
fi

