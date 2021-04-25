BIN=sign_ed_shaX

KDIR=./keys/

make $BIN

./$BIN -p $KDIR/ed_priv_0.bin -b $KDIR/ed_pub_0.bin -s $BIN

exit $?
