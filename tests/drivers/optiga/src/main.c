/*
 * Copyright (c) 2020 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <zephyr.h>

#include <drivers/crypto/optiga.h>

static struct device *dev = NULL;

static const u8_t get_data_object_apdu[] = {
		0x81, /* command code */
		0x00, /* param, read data */
		0x00, 0x02, /* Length */
		0xE0, 0xC2, /* OID of Coprocessor UID */
};

void test_find_chip(void)
{
	dev = device_get_binding("trust-m");
	zassert_not_null(dev, "Device not found");
}

void test_get_chip_id(void)
{
#define TMP_BUF_SIZE 1024
static u8_t tmp_buf[TMP_BUF_SIZE] = {0};

	struct optiga_apdu get_do_txrx = {
		.tx_buf = get_data_object_apdu,
		.tx_len = sizeof(get_data_object_apdu),
		.rx_buf = tmp_buf,
		.rx_len = TMP_BUF_SIZE,
	};

	optiga_enqueue_apdu(dev, &get_do_txrx);

	struct k_poll_event events[1] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
                                 K_POLL_MODE_NOTIFY_ONLY,
                                 &get_do_txrx.finished),
	};

	k_poll(events, 1, K_FOREVER);

	int res = events[0].signal->result;

	zassert_equal(res, 0, "Event returned error code");
	zassert_equal(get_do_txrx.rx_buf[0], 0x00, "APDU error status code");
#undef TMP_BUF_SIZE
}

void test_main(void)
{
	ztest_test_suite(optiga_driver_tests,
		ztest_unit_test(test_find_chip),
		ztest_unit_test(test_get_chip_id)
	);

	ztest_run_test_suite(optiga_driver_tests);
}
