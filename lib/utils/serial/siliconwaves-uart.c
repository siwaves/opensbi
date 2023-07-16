/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Richard Dai <richard@siliconwaves.com>
 */

#include <sbi/riscv_io.h>
#include <sbi/sbi_console.h>
#include <sbi_utils/serial/siliconwaves-uart.h>

/* clang-format off */

#define UART_RBR_OFFSET		0	/* In:  Recieve Buffer Register */
#define UART_THR_OFFSET		0	/* Out: Transmitter Holding Register */
#define UART_DLL_OFFSET		0	/* Out: Divisor Latch Low */
#define UART_IER_OFFSET		1	/* I/O: Interrupt Enable Register */
#define UART_DLM_OFFSET		1	/* Out: Divisor Latch High */
#define UART_FCR_OFFSET		2	/* Out: FIFO Control Register */
#define UART_IIR_OFFSET		2	/* I/O: Interrupt Identification Register */
#define UART_LCR_OFFSET		3	/* Out: Line Control Register */
#define UART_MCR_OFFSET		4	/* Out: Modem Control Register */
#define UART_LSR_OFFSET		5	/* In:  Line Status Register */
#define UART_MSR_OFFSET		6	/* In:  Modem Status Register */
#define UART_SCR_OFFSET		7	/* I/O: Scratch Register */
#define UART_MDR1_OFFSET	8	/* I/O:  Mode Register */

#define UART_LSR_FIFOE		0x80	/* Fifo error */
#define UART_LSR_TEMT		0x40	/* Transmitter empty */
#define UART_LSR_THRE		0x20	/* Transmit-hold-register empty */
#define UART_LSR_BI		0x10	/* Break interrupt indicator */
#define UART_LSR_FE		0x08	/* Frame error indicator */
#define UART_LSR_PE		0x04	/* Parity error indicator */
#define UART_LSR_OE		0x02	/* Overrun error indicator */
#define UART_LSR_DR		0x01	/* Receiver data ready */
#define UART_LSR_BRK_ERROR_BITS	0x1E	/* BI, FE, PE, OE bits */

/* clang-format on */

static volatile void *siliconwaves_uart_base;
static u32 siliconwaves_uart_in_freq;
static u32 siliconwaves_uart_baudrate;
static u32 siliconwaves_uart_reg_width;
static u32 siliconwaves_uart_reg_shift;

static u32 get_reg(u32 num)
{
	u32 offset = num << siliconwaves_uart_reg_shift;

	if (siliconwaves_uart_reg_width == 1)
		return readb(siliconwaves_uart_base + offset);
	else if (siliconwaves_uart_reg_width == 2)
		return readw(siliconwaves_uart_base + offset);
	else
		return readl(siliconwaves_uart_base + offset);
}

static void set_reg(u32 num, u32 val)
{
	u32 offset = num << siliconwaves_uart_reg_shift;

	if (siliconwaves_uart_reg_width == 1)
		writeb(val, siliconwaves_uart_base + offset);
	else if (siliconwaves_uart_reg_width == 2)
		writew(val, siliconwaves_uart_base + offset);
	else
		writel(val, siliconwaves_uart_base + offset);
}

static void siliconwaves_uart_putc(char ch)
{
	while ((get_reg(UART_LSR_OFFSET) & UART_LSR_THRE) == 0)
		;

	set_reg(UART_THR_OFFSET, ch);
}

static int siliconwaves_uart_getc(void)
{
	if (get_reg(UART_LSR_OFFSET) & UART_LSR_DR)
		return get_reg(UART_RBR_OFFSET);
	return -1;
}

static struct sbi_console_device siliconwaves_uart_console = {
	.name = "w3k_uart",
	.console_putc = siliconwaves_uart_putc,
	.console_getc = siliconwaves_uart_getc
};

int siliconwaves_uart_init(unsigned long base, u32 in_freq, u32 baudrate, u32 reg_shift,
		  u32 reg_width, u32 reg_offset)
{
	u16 bdiv;
	siliconwaves_uart_base      = (volatile void *)base + reg_offset;
	siliconwaves_uart_reg_shift = reg_shift;
	siliconwaves_uart_reg_width = reg_width;
	siliconwaves_uart_in_freq   = in_freq;
	siliconwaves_uart_baudrate  = baudrate;

	bdiv = siliconwaves_uart_in_freq / (siliconwaves_uart_baudrate);

	/* Disable all interrupts */
	set_reg(UART_IER_OFFSET, 0x00);
	/* Enable DLAB */
	set_reg(UART_LCR_OFFSET, 0x80);

	if (bdiv) {
		/* Set divisor low byte */
		set_reg(UART_DLL_OFFSET, bdiv & 0xff);
		/* Set divisor high byte */
		set_reg(UART_DLM_OFFSET, (bdiv >> 8) & 0xff);
	}

	/* 8 bits, no parity, one stop bit */
	set_reg(UART_LCR_OFFSET, 0x03);
	/* Enable FIFO */
	set_reg(UART_FCR_OFFSET, 0x01);
	/* No modem control DTR RTS */
	set_reg(UART_MCR_OFFSET, 0x00);
	/* Clear line status */
	get_reg(UART_LSR_OFFSET);
	/* Read receive buffer */
	get_reg(UART_RBR_OFFSET);
	/* Set scratchpad */
	set_reg(UART_SCR_OFFSET, 0x00);

	sbi_console_set_device(&siliconwaves_uart_console);

	return 0;
}
