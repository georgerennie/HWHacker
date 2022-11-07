#include "serial.h"
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/regs/spi.h>
#include <cstdio>

namespace TargetInterfaces {

spi_inst_t* const Serial::spi_intf = spi0;

void Serial::init() {
	// Setup spi peripheral
	spi_init(spi_intf, 10000000);
	spi_set_slave(spi_intf, true);

	spi_set_format(spi_intf, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

	// Setup pin directions
	gpio_set_function(sclk_pin, GPIO_FUNC_SPI);
	gpio_set_function(rx_pin, GPIO_FUNC_SPI);
	// gpio_set_function(tx_pin, GPIO_FUNC_SPI);
	gpio_set_function(cs_n_pin, GPIO_FUNC_SPI);

	// Trigger interrupt when RX fifo gets data
	spi_get_hw(spi_intf)->imsc = spi_get_hw(spi_intf)->imsc | SPI_SSPIMSC_RTIM_BITS;

	auto spi_irq = spi_get_index(spi_intf) == 0 ? SPI0_IRQ : SPI1_IRQ;
	irq_set_exclusive_handler(spi_irq, interrupt_handler);
	irq_set_enabled(spi_irq, true);
}

void Serial::deinit() {
	spi_deinit(spi_intf);

	gpio_set_function(sclk_pin, GPIO_FUNC_SIO);
	gpio_set_function(rx_pin, GPIO_FUNC_SIO);
	gpio_set_function(tx_pin, GPIO_FUNC_SIO);

	gpio_set_dir(sclk_pin, GPIO_IN);
	gpio_set_dir(rx_pin, GPIO_IN);
	gpio_set_dir(tx_pin, GPIO_IN);
}

void Serial::interrupt_handler() {
	while (spi_is_readable(spi_intf)) {
		// Print data register
		// TODO: This should probably end up in a buffer or something idk
		putchar(spi_get_const_hw(spi_intf)->dr);
	}
}

} // namespace TargetInterfaces
