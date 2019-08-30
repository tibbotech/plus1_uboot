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

static inline int s2ull( const char *p, loff_t *num) {
 char *endptr;
 *num = simple_strtoull( p, &endptr, 16);
 return( *p != '\0' && *endptr == '\0');  }

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

int dv_get_mmc_part( const u_char *_pn, int *_nand_devn, loff_t *_off, loff_t *_size,
		loff_t *_maxsize, unsigned char *_part_num) {
// struct mmc *mmc = dv_mmc_start();
// struct blk_desc *bdev;
// int ret = blk_get_device_by_str( "mmc", "0", &bdev);
// if ( ret < 0) return( CMD_RET_FAILURE);
 int part;
 struct blk_desc *dev_desc;
 disk_partition_t info;
 part = blk_get_device_part_str( "mmc", "0:kernel", &dev_desc, &info, 0);
printf( "part name:%s\n", info.name);
 return( part);  }

int tps_upd_w_mmc( ulong _u_data, ulong _u_size, const u_char *_to) {
 int n_devn = 0;
 loff_t n_off = 0, n_sz = 0, n_maxsz = 0;
 size_t w_sz = 0;
 unsigned char part_num = 0;
 int ret = 0;

 printf( "Doing upd from 0x%lX size 0x%lX at %s\n", _u_data, _u_size, _to);

 if ( _to[ 0] == '0' && _to[ 1] == 'x' && !s2ull( ( const char *)_to, &n_off)) {
   printf( "Can't get information about '%s' MMC offset\n", _to);
   return( -1);
 } else if ( dv_get_mmc_part( _to, &n_devn, &n_off, &n_sz, &n_maxsz, &part_num) != 0
 ) {
   printf( "Can't get information about '%s' NAND partition\n", _to);
   return( -1);  }
 //
 printf( "Done upd offset:0x%llX lim:0x%llX, actually wrote:0x%X /ret:%d\n", n_off, n_maxsz, w_sz, ret);
// tps_leds_set( 2);
 return( ret);  }
