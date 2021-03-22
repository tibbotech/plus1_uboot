/* BOOT0 header: uhdr+jump to native reset */
_start:
		.rept 64  /*uhdr*/
		.byte 0
		.endr
		b reset
		.rept 60 /*reserve*/
		.byte 0
		.endr