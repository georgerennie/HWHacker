#include "spi_prog.h"
#include <hardware/gpio.h>
#include <pico/binary_info.h>

namespace TargetInterfaces {

spi_inst_t* const SPIProg::spi_intf = spi0;

void SPIProg::init() {
	// Setup spi peripheral
	spi_init(spi_intf, baudrate);

	spi_set_format(spi_intf, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

	// Setup pin directions
	gpio_set_function(sclk_pin, GPIO_FUNC_SPI);
	gpio_set_function(rx_pin, GPIO_FUNC_SPI);
	gpio_set_function(tx_pin, GPIO_FUNC_SPI);

	// TODO: Come up with some shared way of managing these between the
	// different interfaces so we only reset the chip etc when necessary
	gpio_init(rst_n_pin);
	gpio_set_dir(rst_n_pin, GPIO_OUT);
	gpio_put(rst_n_pin, 1);

	gpio_init(pwr_en_pin);
	gpio_set_dir(pwr_en_pin, GPIO_OUT);
	gpio_put(pwr_en_pin, 0);

	// Make the SPI pin info available to picotool
	bi_decl(bi_3pins_with_func(rx_pin, tx_pin, sclk_pin, GPIO_FUNC_SPI));

	// Make rst_n and power enable pin info available to picotool
	bi_decl(bi_1pin_with_name(rst_n_pin, "AVR Reset_n"));
	bi_decl(bi_1pin_with_name(pwr_en_pin, "AVR Power Enable"));
}

void SPIProg::start_programming() {
	// TODO: Check if already running

	gpio_put(rst_n_pin, 0);
	gpio_put(pwr_en_pin, 1);
	sleep_ms(30);

	transfer_cmd(Cmd::ProgEn);
}

void SPIProg::restart_programming() {
	gpio_put(rst_n_pin, 1);
	sleep_ms(2);
	gpio_put(rst_n_pin, 0);
}

void SPIProg::finish_programming() {
	gpio_put(rst_n_pin, 1);

	gpio_set_function(sclk_pin, GPIO_FUNC_SIO);
	gpio_set_function(rx_pin, GPIO_FUNC_SIO);
	gpio_set_function(tx_pin, GPIO_FUNC_SIO);
}

void SPIProg::write_flash_program(const uint8_t data[], uint16_t len, bool erase) {
	if (erase) {
		transfer_cmd(Cmd::ChipErase);
		wait_for_ready();
	}

	uint16_t pages = (((len / 2) - 1) / flash_page_size) + 1;
	for (uint16_t page = 0; page < pages - 1; page++) {
		write_flash_page(page, &data[page * flash_page_size * 2], flash_page_size);
	}

	uint16_t last_page = pages - 1;
	uint16_t last_idx = last_page * flash_page_size * 2;
	write_flash_page(last_page, &data[last_idx], len - last_idx);
}

// Write len bytes to page page_addr
void SPIProg::write_flash_page(const uint16_t page_addr, const uint8_t data[], uint16_t len) {
	for (uint16_t word = 0; word < len / 2; word++) {
		transfer_cmd(Cmd::LoadPGMPageLow(word, data[2 * word]));
		transfer_cmd(Cmd::LoadPGMPageHigh(word, data[2 * word + 1]));
	}
	transfer_cmd(Cmd::WritePGMPage(flash_page_size * page_addr));
	wait_for_ready();
}

SPIProg::Cmd::T SPIProg::transfer_cmd(const SPIProg::Cmd::T& cmd) {
	Cmd::T result;
	for (;;) {
		spi_write_read_blocking(spi_intf, cmd.data(), result.data(), 4);
		// TODO: should this timeout?
		if (result[2] == cmd[1]) { return result; }
		restart_programming();
	}
}

inline void SPIProg::wait_for_ready() {
	// TODO: should this timeout?
	while (transfer_cmd(Cmd::PollReadyBusy)[3] & 0x01) {}
}

} // namespace TargetInterfaces
