/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/serial/siliconwaves-uart.h>

static int serial_siliconwaves_uart_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	int rc;
	struct platform_uart_data uart;

	rc = fdt_parse_uart8250_node(fdt, nodeoff, &uart);
	if (rc)
		return rc;

	return siliconwaves_uart_init(uart.addr, uart.freq, uart.baud,
			     uart.reg_shift, uart.reg_io_width, uart.reg_offset);
}

static const struct fdt_match serial_siliconwaves_uart_match[] = {
	{ .compatible = "siliconwaves,w3k-uart" },
	{ },
};

struct fdt_serial fdt_serial_siliconwaves = {
	.match_table = serial_siliconwaves_uart_match,
	.init = serial_siliconwaves_uart_init,
};
