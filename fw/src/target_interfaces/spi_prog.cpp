#include "spi_prog.h"
#include <hardware/gpio.h>

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

	gpio_init(rst_n_pin);
	gpio_set_dir(rst_n_pin, GPIO_OUT);

	gpio_init(pwr_en_pin);
	gpio_set_dir(pwr_en_pin, GPIO_OUT);
}

void SPIProg::deinit() {
	spi_deinit(spi_intf);

	gpio_set_function(sclk_pin, GPIO_FUNC_SIO);
	gpio_set_function(rx_pin, GPIO_FUNC_SIO);
	gpio_set_function(tx_pin, GPIO_FUNC_SIO);

	gpio_set_dir(sclk_pin, GPIO_IN);
	gpio_set_dir(rx_pin, GPIO_IN);
	gpio_set_dir(tx_pin, GPIO_IN);

	// gpio_set_dir(rst_n_pin, GPIO_IN);
	// gpio_set_dir(pwr_en_pin, GPIO_IN);
}

void SPIProg::start_programming() {
	// TODO: Check if already running

	gpio_put(rst_n_pin, 0);
	gpio_put(pwr_en_pin, 1);
	sleep_ms(25);

	execute_cmd(Cmd::ProgEn);
}

void SPIProg::restart_programming() {
	gpio_put(rst_n_pin, 1);
	sleep_ms(2);
	gpio_put(rst_n_pin, 0);
	// TODO: Should this transfer ProgEn here
}

void SPIProg::finish_programming() {
	gpio_put(rst_n_pin, 1);
	gpio_put(pwr_en_pin, 0);
}

void SPIProg::program_flash(std::span<const uint8_t> data) {
	execute_cmd(Cmd::ChipErase);
	wait_for_ready();

	const auto pages = (data.size() - 1) / flash_page_bytes + 1;
	for (uint16_t page = 0; page < pages - 1; page++) {
		write_flash_page(page, data.subspan(page * flash_page_bytes, flash_page_bytes));
	}

	const auto last_page = pages - 1;
	write_flash_page(last_page, data.subspan(last_page * flash_page_bytes));
}

uint8_t SPIProg::read_flash_byte(const uint16_t addr) {
	const auto word_addr = addr >> 1;
	if (addr & 0x1) {
		return execute_cmd(Cmd::ReadPGMHigh(word_addr));
	} else {
		return execute_cmd(Cmd::ReadPGMLow(word_addr));
	}
}

void SPIProg::write_flash_page(const uint16_t page_addr, std::span<const uint8_t> data) {
	for (uint16_t word = 0; word < data.size_bytes() / 2; word++) {
		execute_cmd(Cmd::LoadPGMPageLow(word, data[2 * word]));
		execute_cmd(Cmd::LoadPGMPageHigh(word, data[2 * word + 1]));
	}
	execute_cmd(Cmd::WritePGMPage(flash_page_words * page_addr));
	wait_for_ready();
}

uint8_t SPIProg::execute_cmd(const SPIProg::Cmd::T& cmd) {
	for (;;) {
		Cmd::T result;
		spi_write_read_blocking(spi_intf, cmd.data(), result.data(), 4);
		// TODO: should this timeout?
		if (result[2] == cmd[1]) {
			return result[3];
		}
		restart_programming();
	}
}

inline void SPIProg::wait_for_ready() {
	// TODO: should this timeout?
	while (execute_cmd(Cmd::PollReadyBusy) & 0x01) {}
}

} // namespace TargetInterfaces
