#
# (C) Copyright 2012 The Chromium Authors
#
# SPDX-License-Identifier:	GPL-2.0+
#

#OPENSSL=openssl
OPENSSL=../../../../../../build/tools/openssl
HASH=.hash

obj-y += secure_verify.o

rsakey_N.h: pk.pem
	$(OPENSSL) rsa -in pk.pem -text -noout | head -20 | tail -18 | sed -e 's|    00:|    |;s|:|, 0x|g;s|    |0x|g;s|, 0x$$|,|g' > rsakey_N.h

pk.pem:
	$(OPENSSL) genrsa -out pk.pem 2048

sign:
	@rm -f $(HASH)
	$(OPENSSL) sha3-512 -binary $(IMG) > $(HASH)
	@wc -c $(HASH) | grep -q "64 $(HASH)"
	$(OPENSSL) pkeyutl -sign -inkey pk.pem -in $(HASH) >> $(IMG)

clean:
	rm -f pk.pem rsakey_N.h
