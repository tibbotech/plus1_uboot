#include <common.h>
#include <image.h>
#include "sc_api.h"
#include "secure_config.h"
void prn_dump(char* str,unsigned char *buf, int len)
{
	int i;
	printf("%s",str);
	for (i = 0; i < len; i++) {
		if (i && !(i & 0xf)) {
			printf(" \n");
		}
		printf("0x%x ",buf[i]);
	}
	puts(" \n");
}

static int is_nonzero(const u8 *buf, int len)
{
	int i = 0;

	for (i = 0; i < len; i++)
		if (buf[i])
			return 1;
	return 0;
}

const uint8_t ed_pub_0[32] = { 0x5B, 0x1D, 0x18, 0x90, 0x04, 0x36, 0x70, 0x8B, 0x00, 0x8B, 0x2F, 0xB1, 0x12, 0x5F, 0xF6, 0xE8, 0xE6, 0x51, 0x24, 0x8C, 0x45, 0x59, 0x30, 0xCF, 0x2F, 0x46, 0x1C, 0x6A, 0x43, 0xB7, 0x6D, 0x48 };
static int q645_load_otp_Sb_pub_key(u8 in_pub[32])
{
	int ret = 0;

#if 1//test code for use test-keys
	//#include "../secure/test-keys/ed_pub_0.inc"
	memcpy(in_pub, ed_pub_0, 32);
	printf("Test pub-key:\n");
#else
	CSTAMP(0xbbbbbbbb);
	printf("load OTP Sb_Kpub\n");

	ret = SC_key_otp_load(in_pub, 0, 32); // G779.0~7
#endif

#ifdef CONFIG_BOOT_ON_ZEBU
	prn_dump("Kpub:\n", in_pub, 32);
#endif
	return ret;
}

static struct sb_info *q645_get_sb_info(const struct image_header  *hdr)
{
	struct sb_info *xsb = NULL;

	int imgsize=image_get_size(hdr);
	
	// offs_sb is offset to sb_info after uboot_hdr
	xsb =(struct sb_info *)(((u8 *)hdr) + imgsize  + sizeof(struct image_header) - SB_INFO_SIZE);
	if (((u64)xsb & 0x3) || (xsb->magic != SB_MAGIC)) {
		printf("bad sb magic @%p",xsb);
		if (0 == ((u64)xsb & 3)) {
			printf("bad magic value = %x\n",xsb->magic);
			prn_dump("sb_info\n", (unsigned char *)xsb, sizeof(struct sb_info));
		}
		return NULL;
	}
	return xsb;
}

static int q645_verify_uboot_signature(const struct image_header *hdr, struct sb_info *xsb)
{
	__ALIGN4
	u8 h_val[64], sig[64];
	__ALIGN4
	u8 in_pub[32];
	u8 *data;
	int sig_size = 64;
	int data_size;
	u32 t1, t2;
	int ret = -1;
	
	printf("Verify signature\n");
	
	/* Load public key */
	if (q645_load_otp_Sb_pub_key(in_pub)) {
		printf("load otp Sb_Kpub fail\n");
		return ret;
	}

	if (!is_nonzero(in_pub, 32)) {
		printf("Sb_Kpub : all zero\n");
		return -1;
	}

	if (SC_shaX_512(in_pub, 32, (u8 *)h_val)) {
		printf("[%s][%d]\n",__FUNCTION__,__LINE__);
		return -1;
	}

	if (*(u32 *)h_val != xsb->hash_Sb_Kpub) {
		printf("Detected wrong key, Sb_Kpub hash ");
		return ret;
	}

	/* load signature from sb_info */
	memcpy(sig, xsb->sb_signature, 64);
	memset(xsb->sb_signature, 0, 64); /* compute signature with sb_signature = 0 */

	/* data = bin + sb_info */
	data = ((u8 *)hdr) + sizeof(struct image_header);
	data_size = image_get_size(hdr);

	/* ed25519 hash sequence */
	SC_ed25519_hash(sig, data, data_size, in_pub, h_val);
	/* verify signature : EdDSA */
#ifdef FOR_ZEBU_CSIM
	t1 = get_ticks();
#else
	t1 = get_timer(0);
#endif
	ret = SC_ed25519_verify_hash(sig, in_pub, h_val);

#ifdef FOR_ZEBU_CSIM
	t2 = get_ticks();
#else
	t2 = get_timer(t1);
#endif

	if (ret) {
		printf(" FAIL\n");
		prn_dump("signature in image is bad:\n", sig, sig_size);
		prn_dump("calc hash:\n", h_val, sizeof(h_val));
	} else {
		printf(" OK\n");
	}

	printf("tV=%d",t2);

	return ret;
}


// verify_kernel_signature
int do_verify(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;
	struct sb_info *xsb;
	image_header_t  *hdr;
	
	if (argc < 3) 
	{
		return CMD_RET_USAGE;
	}
	
	u32 kernel_addr =  simple_strtoul(argv[1], NULL, 0);
	int compile_with_secure =  simple_strtoul(argv[2], NULL, 0);
	
	if(compile_with_secure == 0){
		printf("not Secure image\n");
		return 0;
	}

	hdr = (image_header_t  *)(u64)kernel_addr;
	printf("\nkernel_hdr addr = %x\n",kernel_addr);
	if(hdr == NULL)
		goto out;
	
	if (!image_check_magic(hdr)) {
		puts("Verify:Bad Magic Number\n");
		goto out;
	}
	
	if (!IS_IC_SECURE_ENABLE()) {
		printf("Error: non-secure IC can't boot Secure image\n");
		return -1;
	}
	/* Secure boot flow requirement:
	 * 1. OTP[RMA] != 0
	 * 2. OTP[SECURE] = 1
	 * 3. OTP[KEY] != 0
	 * 4. SB image (sb_info appended)
	 */

	/* Is SB info appended */
	printf("read SB info\n");
	xsb = q645_get_sb_info(hdr);
	if (NULL == xsb){
		printf("SB img: bad SB info\n");
		goto out;
	}

	/* Is signature appended ? */
	if ((xsb->sb_flags & 0x1) == 0) {
		printf("SB img: missed signed flag\n");
		goto out;
	}

	/* Verify signature */

	if (q645_verify_uboot_signature(hdr, xsb)) {
		// Bad signature
		printf("Bad signature\n");
		// Let boot_flow() fallback to ISP soon
		goto out;
	}
	printf("Verified signature OK\n");
	return 0;

out:
	if(ret)
	{
		printf("veify kernel fail !!");
		while(1);
	}
	return ret;
}


U_BOOT_CMD(
	verify, 3, 1, do_verify,
	"verify command",
	"verify kernel signature.\n"
	"\taddr: kernel addr, include uImage header.\n"
	"\tverify 0x307fc0\n"
);
