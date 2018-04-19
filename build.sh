function build_uboot() {
	export CROSS_COMPILE=armv7hf-glibc-linux-
	make $1
	make clean
	make all -j8
}

# check if toolchain is ok or nor
which armv7hf-glibc-linux-gcc
if [ $? -eq 0 ]; then
	echo "toolchain is ready."
else
	echo ""
	echo "toolchain is not ready!"
	echo "Adding toolchain path to your \$PATH"
	echo ""
	export PATH=$PATH:../../build/tools/armv7-eabihf--glibc--stable/bin
fi

# check if config is ok or not
available_configs=`make list`

for config in $available_configs
do
	if [ ! -z $1 ] && [ $1 = $config ];then
		matched="OK"
		break
	fi
done

# build u-boot
if [ ! -z $1 ] && [ $matched = "OK" ];then
	build_uboot $1
else
	echo "invalid defconfig : [$1]"
	echo "available configs are :"
	echo "$available_configs"
fi
