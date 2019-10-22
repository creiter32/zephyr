/*
 * Copyright (c) 2019 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <kernel.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include "crypto_optiga.h"

#define LOG_LEVEL CONFIG_CRYPTO_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(optiga);

typedef int (*dummy_api_t)();

struct dummy_api {
	dummy_api_t dummy;
};

static const struct dummy_api dummy_funcs = {
	.dummy = NULL,
};

struct optiga_cfg {
	char *i2c_dev_name;
	u16_t i2c_addr;
};

int optiga_reg_read(struct device *dev, u8_t addr, u8_t *data, size_t len)
{
	struct optiga_data *driver = dev->driver_data;
	const struct optiga_cfg *config = dev->config->config_info;

	// select register for write
	bool acked = false;
	int res = 0;
	int i = 0;
	for(i = 0; i < 5; i++) {
		res = i2c_write(driver->i2c_master, &addr, sizeof(addr), config->i2c_addr);
		if (res == 0) {
			acked = true;
			break;
		}
		k_sleep(10);
	}

	if (!acked) {
		LOG_DBG("No ACK for register address received");
		return -EIO;
	}

	LOG_DBG("Reg ACK after %d tries", i);

	acked = false;
	for(i = 0; i < 5; i++) {
		res = i2c_read(driver->i2c_master, data, len, config->i2c_addr);
		if (res == 0) {
			acked = true;
			break;
		}
		k_sleep(10);
	}

	if (!acked) {
		LOG_DBG("No ACK for read data received");
		return -EIO;
	}

	LOG_DBG("Read ACK after %d tries", i);

	return res;
}

int optiga_init(struct device *dev)
{
	LOG_DBG("Init OPTIGA");

	const struct optiga_cfg *config = dev->config->config_info;
	struct optiga_data *data = dev->driver_data;

	data->i2c_master = device_get_binding(config->i2c_dev_name);
	if (data->i2c_master == NULL) {
		LOG_ERR("Failed to get I2C device");
		return -EINVAL;
	}

	return 0;
}

#define OPTIGA_DEVICE(id)						\
	static const struct optiga_cfg optiga_##id##_cfg = {		\
		.i2c_dev_name = DT_INST_##id##_INFINEON_OPTIGA_TRUST_M_BUS_NAME,	\
		.i2c_addr     = DT_INST_##id##_INFINEON_OPTIGA_TRUST_M_BASE_ADDRESS,	\
	};								\
									\
static struct optiga_data optiga_##id##_data;				\
									\
DEVICE_AND_API_INIT(optiga_##id, DT_INST_##id##_INFINEON_OPTIGA_TRUST_M_LABEL,	\
		    &optiga_init, &optiga_##id##_data,		\
		    &optiga_##id##_cfg, POST_KERNEL,			\
		    CONFIG_CRYPTO_INIT_PRIORITY, &dummy_funcs)

#ifdef DT_INST_0_INFINEON_OPTIGA_TRUST_M
OPTIGA_DEVICE(0);
#endif /* DT_INST_0_INFINEON_OPTIGA_TRUST_M */