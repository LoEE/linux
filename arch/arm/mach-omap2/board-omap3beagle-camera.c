/*
 * arch/arm/mach-omap2/board-omap3beagle-camera.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/mach-types.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <plat/cpu.h>
#include <plat/i2c.h>

#include <media/mt9p031.h>
#include <media/omap3isp.h>

#include "devices.h"

#define MT9P031_RESET_GPIO	98

static struct regulator_consumer_supply mt9p031_dummy_supplies[] = {
	REGULATOR_SUPPLY("vaa", "2-0048"),
};

static struct mt9p031_platform_data beagle_mt9p031_platform_data = {
	.reset		= MT9P031_RESET_GPIO,
	.ext_freq	= 21000000,
	.target_freq	= 48000000,
};

static struct i2c_board_info mt9p031_camera_i2c_device = {
	I2C_BOARD_INFO("mt9p031", 0x48),
	.platform_data = &beagle_mt9p031_platform_data,
};

static struct isp_subdev_i2c_board_info mt9p031_camera_subdevs[] = {
	{
		.board_info = &mt9p031_camera_i2c_device,
		.i2c_adapter_id = 2,
	},
	{ NULL, 0, },
};

static struct isp_v4l2_subdevs_group beagle_camera_subdevs[] = {
	{
		.subdevs = mt9p031_camera_subdevs,
		.interface = ISP_INTERFACE_PARALLEL,
		.bus = {
			.parallel = {
				.data_lane_shift = ISP_LANE_SHIFT_0,
				.clk_pol = 1,
			}
		},
	},
	{ },
};

static struct isp_platform_data beagle_isp_platform_data = {
	.xclks = {
		[0] = {
			.dev_id = "2-0048",
		},
	},
	.subdevs = beagle_camera_subdevs,
};

static int __init beagle_camera_init(void)
{
	if (!machine_is_omap3_beagle() || !cpu_is_omap3630())
		return 0;

	clk_add_alias(NULL, "2-0048", "cam_xclka", NULL);

	regulator_register_fixed(0, mt9p031_dummy_supplies,
				 ARRAY_SIZE(mt9p031_dummy_supplies));

	omap_register_i2c_bus(2, 100, NULL, 0);
	omap3_init_camera(&beagle_isp_platform_data);

	return 0;
}
late_initcall(beagle_camera_init);

static int __init beagle_camera_setup(char *p)
{
	if (p == NULL)
		return 0;

	if (strcmp(p, "mt9p031") == 0 || strcmp(p, "mt9p031m") == 0)
		strcpy(mt9p031_camera_i2c_device.type, p);
	else
		printk(KERN_ERR "%s: unknown camera module %s\n", __func__, p);

	return 0;
}
early_param("beagle.camera", beagle_camera_setup);
