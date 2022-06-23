#!/bin/bash

PROG=./aes-gcm

function test_it
{
	# test dat size
	data_len=$1

	# verbose
	vbs=$2

	echo "* Test Data length: $data_len"

	rm -f plain.bin cipher.bin tag.bin

	dd if=/dev/urandom of=iv.bin bs=1 count=12	2>/dev/null
	dd if=/dev/urandom of=key.bin bs=1 count=32	2>/dev/null
	dd if=/dev/urandom of=plain.bin bs=1 count=$data_len 2>/dev/null

	$PROG 1 iv.bin key.bin plain.bin cipher.bin tag.bin $vbs
	if [ $? -ne 0 ];then
		echo "encrypt fail"
		exit 1
	fi
	$PROG 0 iv.bin key.bin cipher.bin restore.bin tag.bin $vbs
	if [ $? -ne 0 ];then
		echo "decrypt fail"
		exit 1
	fi

	cmp plain.bin restore.bin
	if [ $? -ne 0 ];then
		ls -l plain.bin cipher.bin key.bin tag.bin
		echo "Error: mismatched!"
		exit 1
	else
		echo "OK: restore successfully, size=$data_len"
	fi
}

test_it 128
test_it 256
test_it 300
test_it 304
test_it 305
test_it 3050
test_it 30500
test_it 30503
