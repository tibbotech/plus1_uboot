#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <mach/gpio_drv.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_PINS_ONE_IP			128

static int sunplus_pinctrl_config(int offset)
{
	u32 pin_mux[MAX_PINS_ONE_IP];
	int len;
	/*
	 * check for "pinmux" property in each subnode 
	   of pin controller phandle "pinctrl-0"
	 * */
	fdt_for_each_subnode(offset, gd->fdt_blob, offset) {
		int i;
		len = fdtdec_get_int_array_count(gd->fdt_blob, offset,
						 "pinmux", pin_mux,
						 ARRAY_SIZE(pin_mux));
		if (len < 0)
			return -EINVAL;

		for (i = 0; i < len; i++) {
			if(i%2 ==1)
			{
				//printf("pinmux = 0x%08x 0x%08x\n", *(pin_mux + i-1),*(pin_mux + i));
				GPIO_PIN_MUX_SEL(*(pin_mux + i-1),*(pin_mux + i));
			}
		}
	}

	return 0;
}

static int sunplus_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	return sunplus_pinctrl_config(dev_of_offset(config));
}

static struct pinctrl_ops sunplus_pinctrl_ops = {
	.set_state		= sunplus_pinctrl_set_state,
};

static const struct udevice_id sunplus_pinctrl_ids[] = {
 { .compatible = "sunplus,sppctl"},
 { /* zero */ }
};

U_BOOT_DRIVER(pinctrl_sunplus) = {
	.name		= "pinctrl_sunplus",
	.id			= UCLASS_PINCTRL,
	.of_match	= sunplus_pinctrl_ids,
	.ops		= &sunplus_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
};
