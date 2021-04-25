MKEY=mkey_x25519
BIN=x25519_ss

KDIR=./keys/

echo "gen 2 pair of ASYM key"
./$MKEY 2


echo "ss1 = exchange ( priv 0, pub 1)"
./$BIN -p $KDIR/x_priv_0.bin -b $KDIR/x_pub_1.bin -o ss1.bin

echo "ss2 = exchange ( priv 1, pub 0)"
./$BIN -p $KDIR/x_priv_1.bin -b $KDIR/x_pub_0.bin -o ss2.bin

cmp ss1.bin ss2.bin
if [ $? -ne 0 ];then
	echo "FAIL: mismatched shared secret, ss1 != ss2"
	exit 1
else
	echo "OK: ss1 == ss2"
fi

rm -f ss1.bin ss2.bin
