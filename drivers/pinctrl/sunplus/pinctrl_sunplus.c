#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>

#include "pinctrl_sunplus.h"
#ifdef CONFIG_PINCTRL_SUNPLUS
#include <dt-bindings/pinctrl/sppctl-sp7021.h>
#elif defined (CONFIG_PINCTRL_SUNPLUS_Q645)
#include <dt-bindings/pinctrl/sppctl-q645.h>
#elif defined (CONFIG_PINCTRL_SUNPLUS_SP7350)
#include <dt-bindings/pinctrl/sppctl-sp7350.h>
#else
#include <dt-bindings/pinctrl/sppctl-i143.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

//#define PINCTRL_DEBUG

volatile u32 *moon1_regs = NULL;
#ifdef SUPPORT_PINMUX
volatile u32 *moon2_regs = NULL;
#endif
volatile u32 *gpioxt_regs = NULL;
#ifdef CONFIG_PINCTRL_SUNPLUS
volatile u32 *gpioxt2_regs = NULL;
#endif
volatile u32 *first_regs = NULL;
void* pin_registered_by_udev[MAX_PINS];

#ifdef PINCTRL_DEBUG
void pinmux_grps_dump(void)
{
	int i = 0, mask, rval, val;

	struct func_t *func = &list_funcs[i];
	for (i = 0; i < list_funcsSZ; i++) {
		func = &(list_funcs[i]);

		if (func->gnum == 0)
			continue;

		if (func->freg != fOFF_G)
			continue;

		mask = (1 << func->blen) - 1;
		rval = GPIO_PINGRP(func->roff);
		val = (rval >> func->boff) & mask;
		if (val == 0)
			continue;

		printf("%s\t=%d regval:%X\n", list_funcs[i].name, val, rval);
	}
}

#ifdef SUPPORT_PINMUX
void pinmux_reg_dump(void)
{
	u32 pinmux_value[120];
	int i;

	printf("pinmux_reg_dump \n");
	for (i = MUXF_L2SW_CLK_OUT; i <= MUXF_GPIO_INT7; i++)
		sp_gpio_pin_mux_get(i, &pinmux_value[i-MUXF_L2SW_CLK_OUT]);

	printf("l2sw = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
		pinmux_value[0], pinmux_value[1], pinmux_value[2], pinmux_value[3], pinmux_value[4],
		pinmux_value[5], pinmux_value[6], pinmux_value[7], pinmux_value[8], pinmux_value[9]);
	printf("l2sw = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
		pinmux_value[10], pinmux_value[11], pinmux_value[12], pinmux_value[13], pinmux_value[14],
		pinmux_value[15], pinmux_value[16], pinmux_value[17], pinmux_value[18], pinmux_value[19]);
	printf("l2sw = 0x%02x 0x%02x ---- ---- ---- ---- ---- ---- ---- ---- \n",
		pinmux_value[20], pinmux_value[21]);
	printf("sdio = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ---- ---- ---- ---- \n",
		pinmux_value[22], pinmux_value[23], pinmux_value[24], pinmux_value[25], pinmux_value[26],
		pinmux_value[27]);
	printf("pwm  = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ---- ---- \n",
		pinmux_value[28], pinmux_value[29], pinmux_value[30], pinmux_value[31], pinmux_value[32],
		pinmux_value[33], pinmux_value[34], pinmux_value[35]);
	printf("icm  = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ---- ---- \n",
		pinmux_value[36], pinmux_value[37], pinmux_value[38], pinmux_value[39], pinmux_value[40],
		pinmux_value[41], pinmux_value[42], pinmux_value[43]);
	printf("spim = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
		pinmux_value[44], pinmux_value[45], pinmux_value[46], pinmux_value[47], pinmux_value[48],
		pinmux_value[49], pinmux_value[50], pinmux_value[51], pinmux_value[52], pinmux_value[53]);
	printf("spim = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
		pinmux_value[54], pinmux_value[55], pinmux_value[56], pinmux_value[57], pinmux_value[58],
		pinmux_value[59], pinmux_value[60], pinmux_value[61], pinmux_value[62], pinmux_value[63]);
	printf("spis = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
		pinmux_value[64], pinmux_value[65], pinmux_value[66], pinmux_value[67], pinmux_value[68],
		pinmux_value[69], pinmux_value[70], pinmux_value[71], pinmux_value[72], pinmux_value[73]);
	printf("spis = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
		pinmux_value[74], pinmux_value[75], pinmux_value[76], pinmux_value[77], pinmux_value[78],
		pinmux_value[79], pinmux_value[80], pinmux_value[81], pinmux_value[82], pinmux_value[83]);
	printf("i2c  = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ---- ---- \n",
		pinmux_value[84], pinmux_value[85], pinmux_value[86], pinmux_value[87], pinmux_value[88],
		pinmux_value[89], pinmux_value[90], pinmux_value[91]);
	printf("uart = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
		pinmux_value[92], pinmux_value[93], pinmux_value[94], pinmux_value[95], pinmux_value[96],
		pinmux_value[97], pinmux_value[98], pinmux_value[99], pinmux_value[100], pinmux_value[101]);
	printf("uart = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ---- ---- ---- ---- \n",
		pinmux_value[102], pinmux_value[103], pinmux_value[104], pinmux_value[105], pinmux_value[106],
		pinmux_value[107]);
	printf("tim  = 0x%02x 0x%02x 0x%02x 0x%02x ---- ---- ---- ---- ---- ---- \n",
		pinmux_value[108], pinmux_value[109], pinmux_value[110], pinmux_value[111]);
	printf("int  = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ---- ---- \n",
		pinmux_value[112], pinmux_value[113], pinmux_value[114], pinmux_value[115], pinmux_value[116],
		pinmux_value[117], pinmux_value[118], pinmux_value[119]);
}
#endif

void gpio_reg_dump(void)
{
	u32 gpio_value[MAX_PINS];
	int i;

	printf("gpio_reg_dump (FI MA IN II OU OI OE OD)\n");
	for (i = 0; i < MAX_PINS; i++) {
		gpio_value[i] = sp_gpio_para_get(i);
		if ((i%8) == 7) {
			printf("GPIO_P%-2d 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",(i/8),
			gpio_value[i-7],gpio_value[i-6],gpio_value[i-5],gpio_value[i-4],
			gpio_value[i-3],gpio_value[i-2],gpio_value[i-1],gpio_value[i]);
		} else if (i == (MAX_PINS-1)) {
#ifdef CONFIG_PINCTRL_SUNPLUS
			printf("GPIO_P%-2d 0x%02x 0x%02x 0x%02x\n",(i/8),
			gpio_value[i-2],gpio_value[i-1],gpio_value[i]);
#else
			printf("GPIO_P%-2d 0x%02x 0x%02x 0x%02x 0x%02x\n",(i/8),
			gpio_value[i-3],gpio_value[i-2],gpio_value[i-1],gpio_value[i]);
#endif
		}
	}
}
#endif

static void* sp_register_pin(int pin, struct udevice *dev)
{
	// Check if pin number is within range.
	if ((pin >= 0) && (pin < MAX_PINS)) {
		if (pin_registered_by_udev[pin] == 0) {
			// If the pin has not been registered, register it by
			// save a pointer to the registering device.
			pin_registered_by_udev[pin] = dev;
			pctl_info("Register pin %d (from node: %s).\n", pin, dev->name);

			return 0;
		} else {
			pctl_err("ERROR: Pin %d of node %s has been registered (by node: %s)!\n",
				 pin, dev->name, ((struct udevice*)pin_registered_by_udev[pin])->name);
			return dev;
		}
	} else {
		pctl_err("ERROR: Invalid pin number %d from '%s'!\n", pin, dev->name);
		return (void*)-1;
	}
}

static int sp_unregister_pin(int pin)
{
	// Check if pin number is within range.
	if ((pin >= 0) && (pin < MAX_PINS))
		if (pin_registered_by_udev[pin]) {
			pin_registered_by_udev[pin] = 0;
			return 0;
		}

	return -1;
}

static int sp_pinctrl_pins(struct udevice *dev)
{
	int offset = dev_of_offset(dev);
	u32 pin_mux[MAX_PINS];
	int len, i;
#ifdef SUPPORT_PINMUX
	u32 val;
#endif

	// Get property: "sunplus,pins"
	len = fdtdec_get_int_array_count(gd->fdt_blob, offset, "sunplus,pins",
					 pin_mux, ARRAY_SIZE(pin_mux));
	if (len > 0)
		goto found_pins;

	// Get property: "sppctl,pins"
	len = fdtdec_get_int_array_count(gd->fdt_blob, offset, "sppctl,pins",
					 pin_mux, ARRAY_SIZE(pin_mux));
	if (len > 0)
		goto found_pins;

	// Get property: "pins"
	len = fdtdec_get_int_array_count(gd->fdt_blob, offset, "pins",
					 pin_mux, ARRAY_SIZE(pin_mux));
	if (len <= 0)
		return 0;

found_pins:
	pctl_info("Number of entries of 'pins' = %d\n", len);

	// Register all pins.
	for (i = 0; i < len; i++) {
		int pin  = (pin_mux[i] >> 24) & 0xff;

		if (sp_register_pin(pin, dev) != 0)
			break;
	}

	// If any pin was not registered successfully, return -1.
	if ((len > 0) && (i != len))
		return -1;

	// All pins were registered successfully, set up all pins.
	for (i = 0; i < len; i++) {
		int pins = pin_mux[i];
		int pin  = (pins >> 24) & 0xff;
		int type = (pins >> 16) & 0xff;
#ifdef SUPPORT_PINMUX
		int func = (pins >> 8) & 0xff;
#endif
		int flag = pins & 0xff;
		pctl_info("pins = 0x%08x\n", pins);

#ifdef SUPPORT_PINMUX
		if (type == SPPCTL_PCTL_G_PMUX) {
			// It's a PinMux pin.
			sp_gpio_pin_mux_set(func, pin);
			sp_gpio_first_set(pin, 0);
			sp_gpio_master_set(pin, 1);
			sp_gpio_pin_mux_get(func, &val);
			pctl_info("pinmux get = 0x%02x \n", val);
		} else
#endif
		if (type == SPPCTL_PCTL_G_IOPP) {
			// It's a IOP pin.
			sp_gpio_first_set(pin, 1);
			sp_gpio_master_set(pin, 0);
		} else if (type == SPPCTL_PCTL_G_GPIO) {
			// It's a GPIO pin.
			sp_gpio_first_set(pin, 1);
			sp_gpio_master_set(pin, 1);

			if (flag & (SPPCTL_PCTL_L_OUT|SPPCTL_PCTL_L_OU1)) {
				if (flag & SPPCTL_PCTL_L_OUT)
					sp_gpio_out_set(pin, 0);

				if (flag & SPPCTL_PCTL_L_OU1)
					sp_gpio_out_set(pin, 1);

				sp_gpio_oe_set(pin, 1);
			} else if (flag & SPPCTL_PCTL_L_ODR) {
				sp_gpio_open_drain_set(pin, 1);
			}

			if (flag & SPPCTL_PCTL_L_INV)
				sp_gpio_input_invert_set(pin, 1);
			else
				sp_gpio_input_invert_set(pin, 0);

			if (flag & SPPCTL_PCTL_L_ONV)
				sp_gpio_output_invert_set(pin, 1);
			else
				sp_gpio_output_invert_set(pin, 0);
		}
	}

	return 0;
}

static int sp_pinctrl_zero(struct udevice *dev)
{
	int offset = dev_of_offset(dev);
	u32 pin_mux[MAX_PINS];
	int len, i, mask;
#ifdef SUPPORT_PINMUX
	u32 val;
#endif

	// Get property: "sunplus,zerofunc"
	len = fdtdec_get_int_array_count(gd->fdt_blob, offset, "sunplus,zerofunc",
					 pin_mux, ARRAY_SIZE(pin_mux));
	if (len > 0)
		goto found_zero_func;

	// Get property: "sppctl,zero_func"
	len = fdtdec_get_int_array_count(gd->fdt_blob, offset, "sppctl,zero_func",
					 pin_mux, ARRAY_SIZE(pin_mux));
	if (len > 0)
		goto found_zero_func;

	// Get property: "zero_func"
	len = fdtdec_get_int_array_count(gd->fdt_blob, offset, "zero_func",
					 pin_mux, ARRAY_SIZE(pin_mux));
	if (len <= 0)
		return 0;

found_zero_func:
	pctl_info("Number of entries of 'zero_func' = %d\n", len);

	// All pins were registered successfully, set up all pins.
	for (i = 0; i < len; i++) {
		int func = pin_mux[i];
		pctl_info("zero_func = 0x%08x\n", func);

		// Set it to no use.
		if (func >= list_funcsSZ) {
			pctl_info("func=%d is out of range, skipping...\n", func);
			continue;
		}

		struct func_t *f = &list_funcs[func];
		switch (f->freg) {
#ifdef SUPPORT_PINMUX
		case fOFF_M:
			sp_gpio_pin_mux_set(func, 0);
			sp_gpio_pin_mux_get(func, &val);
			pctl_info("pinmux get = 0x%02x\n", val);
			break;
#endif
		case fOFF_G:
			mask = (1 << f->blen) - 1;
			GPIO_PINGRP(f->roff) = (mask << (f->boff+16)) | (0 << f->boff);
			pctl_info("group %s set to 0\n", f->name);
			break;

		default:
			printf("bad zero func/group idx:%d, skipped\n", func);
			break;
		}
	}

	return 0;
}

static int sp_pinctrl_function(struct udevice *dev)
{
	int offset = dev_of_offset(dev);
	const char *pin_group;
	const char *pin_func;
	int len, i;

	// Get property: 'sppctl,function'
	pin_func = fdt_getprop(gd->fdt_blob, offset, "sppctl,function", &len);
	if (pin_func)
		goto found_function;

	// Get property: 'function'
	pin_func = fdt_getprop(gd->fdt_blob, offset, "function", &len);
	if (!pin_func)
		return 0;

found_function:
	pctl_info("function = %s (%d)\n", pin_func, len);
	if (len > 1) {
		// Find 'function' string in list: only groups
		for (i = 0; i < list_funcsSZ; i++) {
			if (list_funcs[i].gnum == 0)
				continue;

			if (list_funcs[i].freg != fOFF_G)
				continue;

			if (strcmp(pin_func, list_funcs[i].name) == 0)
				break;
		}
		if (i == list_funcsSZ) {
			pctl_err("Error: Invalid 'function' in node %s! "
				 "Cannot find \"%s\"!\n", dev->name, pin_func);
			return -1;
		}

		// 'function' is found! Next, find its 'groups'.
		// Get property: 'sppctl,groups'
		pin_group = fdt_getprop(gd->fdt_blob, offset, "sppctl,groups", &len);
		if (pin_group)
			goto found_groups;

		// Get property: 'groups'
		pin_group = fdt_getprop(gd->fdt_blob, offset, "groups", &len);

found_groups:
		pctl_info("groups = %s (%d)\n", pin_group, len);
		if (len > 1) {
			struct func_t *func = &list_funcs[i];

			// Find 'pin_group' string in list.
			for (i = 0; i < func->gnum; i++)
				if (strcmp (pin_group, func->grps[i].name) == 0)
					break;

			if (i == func->gnum) {
				pctl_err("Error: Invalid 'groups' in node %s! "
					 "Cannot find \"%s\"!\n", dev->name, pin_group);
				return -1;
			}

			// 'pin_group' is found!
			const struct sppctlgrp_t *grp = &func->grps[i];

			// Register all pins of the group.
			for (i = 0; i < grp->pnum; i++)
				if (sp_register_pin(grp->pins[i], dev) != 0)
					break;

			// Check if all pins of the group register successfully
			if (i == grp->pnum) {
				int mask;

				// All pins of the group was registered successfully.
				// Enable the pin-group.
				mask = (1 << func->blen) - 1;
				GPIO_PINGRP(func->roff) = (mask<<(func->boff+16)) | (grp->gval<<func->boff);
				pctl_info("GPIO_PINGRP[%d] <= 0x%08x\n", func->roff,
					(mask<<(func->boff+16)) | (grp->gval<<func->boff));
			} else {
				return -1;
			}
		} else if (len <= 1) {
			pctl_err("Error: Invalid 'groups' in node %s!\n", dev->name);
			return -1;
		}
	} else if (len == 1) {
		pctl_err("Error: Invalid 'function' in node %s!\n", dev->name);
		return -1;
	}

	return 0;
}

static int sp_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	int ret;

	pctl_info("\nConfig node: %s\n", config->name);
	ret = sp_pinctrl_pins(config);
	if (ret != 0)
		return ret;

	sp_pinctrl_zero(config);

	ret = sp_pinctrl_function(config);
	if (ret != 0)
		return ret;

#ifdef PINCTRL_DEBUG
#ifdef SUPPORT_PINMUX
	pinmux_reg_dump();
#endif
	gpio_reg_dump();
	pinmux_grps_dump();
#endif

	return 0;
}

static struct pinctrl_ops sunplus_pinctrl_ops = {
	.set_state = sp_pinctrl_set_state,
};

static int sp_gpio_request(struct udevice *dev, unsigned int offset, const char *label)
{
	int err;

	pctl_info("%s: offset = %u, label = %s\n", __func__, offset, label);

	err = sp_gpio_first_set(offset, 1);
	if (err)
		return err;

	err = sp_gpio_master_set(offset, 1);
	if (err)
		return err;

	return 0;
}

static int sp_gpio_rfree(struct udevice *dev, unsigned int offset)
{
	pctl_info("%s: offset = %u\n", __func__, offset);

	//sp_gpio_first_set(offset, 0);
	//sp_gpio_master_set(offset, 0);

	return 0;
}

static int sp_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	u32 val;
	int err;

	pctl_info("%s: offset = %u\n", __func__, offset);

	err = sp_gpio_in(offset, &val);
	if (err)
		return err;

	return !!val;
}

static int sp_gpio_set_value(struct udevice *dev, unsigned int offset, int val)
{
	pctl_info("%s: offset = %u, val = %d\n", __func__, offset, val);

	return sp_gpio_out_set(offset, !!val);
}

static int sp_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	u32 val;
	int err;

	pctl_info("%s: offset = %u\n", __func__, offset);

	err = sp_gpio_oe_get(offset, &val);
	if (err)
		return err;

	return val ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int sp_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	pctl_info("%s: offset = %u\n", __func__, offset);

	return sp_gpio_oe_set(offset, 0);
}

static int sp_gpio_direction_output(struct udevice *dev, unsigned int offset, int val)
{
	pctl_info("%s: offset = %u, val = %d\n", __func__, offset, val);

	sp_gpio_out_set(offset, val);

	return sp_gpio_oe_set(offset, 1);
}

static int sp_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = "GPIO";
	uc_priv->gpio_count = MAX_PINS;

	return 0;
}

static const struct dm_gpio_ops sp_gpio_ops = {
	.request = sp_gpio_request,
	.rfree = sp_gpio_rfree,
	.set_value = sp_gpio_set_value,
	.get_value = sp_gpio_get_value,
	.get_function = sp_gpio_get_function,
	.direction_input = sp_gpio_direction_input,
	.direction_output = sp_gpio_direction_output,
};

static struct driver sp_gpio_driver = {
	.name  = "sunplus_gpio",
	.id    = UCLASS_GPIO,
	.probe = sp_gpio_probe,
	.ops   = &sp_gpio_ops,
};

static int sunplus_pinctrl_probe(struct udevice *dev)
{
	struct udevice *cdev;
	ofnode node;
	int i;

#ifdef SUPPORT_PINMUX
	// Get address of MOON2 registers.
	moon2_regs = (void*)devfdt_get_addr_name(dev, "moon2");
	pctl_info("moon2_regs = %px\n", moon2_regs);
	if (moon2_regs == (void*)-1) {
		pctl_err("Failed to get base address of MOON2!\n");
		return -EINVAL;
	}
#endif

	// Get address of GPIOXT registers.
	gpioxt_regs = (void*)devfdt_get_addr_name(dev, "gpioxt");
	pctl_info("gpioxt_regs = %px\n", gpioxt_regs);
	if (gpioxt_regs == (void*)-1) {
		pctl_err("Failed to get base address of GPIOXT!\n");
		return -EINVAL;
	}

#ifdef CONFIG_PINCTRL_SUNPLUS
	// Get address of GPIOXT2 registers.
	gpioxt2_regs = (void*)devfdt_get_addr_name(dev, "gpioxt2");
	pctl_info("gpioxt2_regs = %px\n", gpioxt2_regs);
	if (gpioxt2_regs == (void*)-1)
		gpioxt2_regs = gpioxt_regs + 0x80;
#endif

	// Get address of FIRST registers.
	first_regs = (void*)devfdt_get_addr_name(dev, "first");
	pctl_info("first_regs = %px\n", first_regs);
	if (first_regs == (void*)-1) {
		pctl_err("Failed to get base address of FIRST!\n");
		return -EINVAL;
	}

	// Get address of MOON1 registers.
	moon1_regs = (void*)devfdt_get_addr_name(dev, "moon1");
	pctl_info("moon1_regs = %px\n", moon1_regs);
	if (moon1_regs == (void*)-1) {
		pctl_err("Failed to get base address of MOON1!\n");
		return -EINVAL;
	}

	// Unregister all pins.
	for (i = 0; i < MAX_PINS; i++)
		sp_unregister_pin(i);

	// Bind gpio driver.
	node = dev_ofnode(dev);
	if (ofnode_read_bool(node, "gpio-controller")) {
		if (!lists_uclass_lookup(UCLASS_GPIO)) {
			pctl_err("Cannot find GPIO driver\n");
			return -ENOENT;
		}

		return device_bind(dev, &sp_gpio_driver, "sunplus_gpio",
				   0, node, &cdev);
	}

	return 0;
}

static const struct udevice_id sunplus_pinctrl_ids[] = {
#ifdef CONFIG_PINCTRL_SUNPLUS
	{ .compatible = "sunplus,sp7021-pctl" },
#elif defined (CONFIG_PINCTRL_SUNPLUS_Q645)
	{ .compatible = "sunplus,q645-pctl" },
#elif defined (CONFIG_PINCTRL_SUNPLUS_SP7350)
	{ .compatible = "sunplus,sp7350-pctl" },
#else
	{ .compatible = "sunplus,i143-pctl" },
#endif
	{ /* zero */ }
};

U_BOOT_DRIVER(pinctrl_sunplus) = {
	.name     = "sunplus_pinctrl",
	.id       = UCLASS_PINCTRL,
	.probe    = sunplus_pinctrl_probe,
	.of_match = sunplus_pinctrl_ids,
	.ops      = &sunplus_pinctrl_ops,
	.bind     = dm_scan_fdt_dev,
};
