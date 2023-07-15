/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Richard Dai <richard@siliconwaves.com>
 */

#ifndef __SERIAL_SILICONWAVES_UART_H__
#define __SERIAL_SILICONWAVES_UART_H__

#include <sbi/sbi_types.h>

int siliconwaves_uart_init(unsigned long base, u32 in_freq, u32 baudrate, u32 reg_shift,
		  u32 reg_width, u32 reg_offset);

#endif
