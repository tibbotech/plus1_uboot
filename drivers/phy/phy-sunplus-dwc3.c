// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Sunplus, Inc.
 *
 * DWC3 Phy driver
 *
 * Author:ChingChouHuang<chingchou.huang@sunplus.com>
 */

#include <dm.h>

static const struct udevice_id sunplus_dwc3phy_ids[] = {
	{ .compatible = "sunplus,usb3-phy" },
	{ }
};

U_BOOT_DRIVER(sunplus_dwc3_phy) = {
	.name	= "phy-sunplus-dwc3",
	.id	= UCLASS_PHY,
	.of_match = sunplus_dwc3phy_ids,
};
