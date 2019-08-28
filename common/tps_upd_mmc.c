/*
 * (C) Copyright 2017 Tibbo Technology
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
#include <mmc.h>
#include <fs.h>
#include <malloc.h>

extern ulong load_addr;

static struct mmc * dv_mmc_start( void) {
 struct mmc *mmc;
 if ( !( mmc = find_mmc_device( 0))) {
   printf( "no mmc device at slot %x\n", 0);
   return( NULL);  }
// printf( "MMC 0 found\n");
 if ( mmc_init( mmc)) {
   printf( "mmc init failed\n");
   return( NULL);  }
// printf( "MMC init done\n");
 return( mmc);  }

int tps_upd_r_mmc( char *filename, ulong addr) {
 int rv = -1;
 if ( dv_mmc_start() == NULL) return( 1);
 // ( cmdtp, flag, argc, )
 char *argv[ 5], addr_buf[ 100];
 memset( addr_buf, 0, 100);
 sprintf( addr_buf, "%lx", addr);
 argv[ 0] = "tps_upd";
 argv[ 1] = "mmc"; // dev
 argv[ 2] = "0:1"; // partition
 argv[ 3] = addr_buf;
 argv[ 4] = filename;
 rv = do_load( NULL, 0, 5, argv, FS_TYPE_ANY);
 return( rv);  }
