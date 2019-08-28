/*
 * (C) Copyright 2016 Tibbo Technology
 *
 * Written by: Dvorkin Dmitry <dvorkin@tibbo.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * based on common/update.c, cmd/usb.c
 */

#include <common.h>

//#if !(defined(CONFIG_FIT) && defined(CONFIG_OF_LIBFDT))
//#error "CONFIG_FIT and CONFIG_OF_LIBFDT are required for auto-update feature"
//#endif
#if !defined(CONFIG_OF_LIBFDT)
#error "CONFIG_OF_LIBFDT are required for auto-update feature"
#endif

#include <command.h>
#include <usb.h>
#include <fs.h>
#include <malloc.h>

extern ulong load_addr;

static int dv_usb_start( void) {
 int ret = -1;
 extern char usb_started;
 if ( !usb_started && usb_init() < 0) return( -1);
#ifndef CONFIG_DM_USB
#ifdef CONFIG_USB_STORAGE
 ret = usb_stor_scan(1);
 if ( ret >= 0) return( ret);
#endif
#endif /* !CONFIG_DM_USB */
#ifdef CONFIG_USB_HOST_ETHER
#ifdef CONFIG_DM_ETH
#ifndef CONFIG_DM_USB
#error "You must use CONFIG_DM_USB if you want to use CONFIG_USB_HOST_ETHER with CONFIG_DM_ETH"
#endif
#else
 ret = usb_host_eth_scan(1);
 if ( ret >= 0) return( ret);
#endif
#endif
 return( -1);  }

int tps_upd_r_usb( char *filename, ulong addr) {
 int rv = 0;

 if ( dv_usb_start() < 0) {
   printf( "Can't find USB storage or ethernet device\n");
   return( 1);  }

 // ( cmdtp, flag, argc, )
 char *argv[ 5], addr_buf[ 100];
 memset( addr_buf, 0, 100);
 sprintf( addr_buf, "%lx", addr);
 argv[ 0] = "tps_upd";
 argv[ 1] = "usb"; // dev
 argv[ 2] = "0:1"; // partition
 argv[ 3] = addr_buf;
 argv[ 4] = filename;
 rv = do_load( NULL, 0, 5, argv, FS_TYPE_ANY);
 return( rv);  }
