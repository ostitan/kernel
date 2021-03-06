/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file Header for I2C driver for Quark SE Sensor Subsystem
 */

#ifndef __DRIVERS_I2C_QUARK_SE_SS_H__
#define __DRIVERS_I2C_QUARK_SE_SS_H__

#include <i2c.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* dev->state values from IC_DATA_CMD Data transfer mode settings (bit 8) */
#define I2C_QSE_SS_STATE_READY		(0)
#define I2C_QSE_SS_CMD_SEND		(1 << 0)
#define I2C_QSE_SS_CMD_RECV		(1 << 1)
#define I2C_QSE_SS_CMD_ERROR		(1 << 2)
#define I2C_QSE_SS_BUSY			(1 << 3)

/*
 * DesignWare speed values don't directly translate from the Zephyr speed
 * selections in include/i2c.h so here we do a little translation
 */
#define I2C_QSE_SS_SPEED_STANDARD	0x1
#define I2C_QSE_SS_SPEED_FAST		0x2
#define I2C_QSE_SS_SPEED_FAST_PLUS	0x2

/* IC_CON Low count and high count default values */
/* TODO verify all values for Quark SE SS */
#define I2C_STD_HCNT			(CONFIG_I2C_CLOCK_SPEED * 4)
#define I2C_STD_LCNT			(CONFIG_I2C_CLOCK_SPEED * 5)
#define I2C_FS_HCNT			((CONFIG_I2C_CLOCK_SPEED * 6) / 8)
#define I2C_FS_LCNT			((CONFIG_I2C_CLOCK_SPEED * 7) / 8)

#define I2C_QSE_SS_FIFO_DEPTH		8

typedef void (*i2c_qse_ss_cfg_func_t)(struct device *port);

struct i2c_qse_ss_rom_config {
	uint32_t		base_address;

	i2c_qse_ss_cfg_func_t	config_func;
};

struct i2c_qse_ss_dev_config {
	device_sync_call_t	sync;
	union dev_config	app_config;

	volatile uint8_t	state;  /* last direction of transfer */

	uint8_t			xfr_flags;

	uint8_t			*xfr_buf;
	uint32_t		xfr_len;
	uint32_t		request_bytes;
	uint32_t		rx_pending;

	uint16_t		hcnt;
	uint16_t		lcnt;
};

#ifdef __cplusplus
}
#endif

#endif /* __DRIVERS_I2C_QUARK_SE_SS_H__ */
