#
# (C) Copyright 2012 The Chromium Authors
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += secure_verify.o

rsakey_N.h: pk.pem
	openssl rsa -in pk.pem -text -noout | head -20 | tail -18 | sed -e 's|    00:|    |;s|:|, 0x|g;s|    |0x|g;s|, 0x$$|,|g' > rsakey_N.h

pk.pem:
	openssl genrsa -out pk.pem 2048

sign:
	openssl sha3-512 -binary $(IMG) | openssl rsautl -sign -inkey pk.pem >> $(IMG)

clean:
	rm -f pk.pem rsakey_N.h
