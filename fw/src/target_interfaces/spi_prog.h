// A minimal implementation of the ATtiny SPI programming protocol needed for
// programming the target AVR chip

#pragma once

#include <hardware/spi.h>
#include <array>
#include <cinttypes>
#include <span>
#include "pins.h"

namespace TargetInterfaces {

class SPIProg {
public:
	SPIProg() { init(); }
	~SPIProg() { deinit(); }
	void init();
	void deinit();

	void start_programming();
	void restart_programming();
	void finish_programming();

	struct Cmd {
		using T                          = std::array<uint8_t, 4>;
		static constexpr T ProgEn        = {0xAC, 0x53, 0x00, 0x00};
		static constexpr T ChipErase     = {0xAC, 0x80, 0x00, 0x00};
		static constexpr T PollReadyBusy = {0xF0, 0x00, 0x00, 0x00};

		static constexpr T LoadPGMPageHigh(const uint16_t word, const uint8_t data) {
			return {0x48, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), data};
		}

		static constexpr T LoadPGMPageLow(const uint16_t word, const uint8_t data) {
			return {0x40, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), data};
		}

		static constexpr T WritePGMPage(const uint16_t page) {
			return {0x4C, static_cast<uint8_t>(page >> 8), static_cast<uint8_t>(page), 0x00};
		}

		static constexpr T ReadPGMHigh(const uint16_t word) {
			return {0x28, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), 0x00};
		}

		static constexpr T ReadPGMLow(const uint16_t word) {
			return {0x20, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), 0x00};
		}

		static constexpr T ReadLFuse = {0x50, 0x00, 0x00, 0x00};
		static constexpr T ReadHFuse = {0x58, 0x08, 0x00, 0x00};
		static constexpr T ReadEFuse = {0x50, 0x08, 0x00, 0x00};

		static constexpr T WriteLFuse(const uint8_t val) { return {0xAC, 0xA0, 0x00, val}; };
		static constexpr T WriteHFuse(const uint8_t val) { return {0xAC, 0xA8, 0x00, val}; };
		static constexpr T WriteEFuse(const uint8_t val) { return {0xAC, 0xA4, 0x00, val}; };
	};

	uint8_t execute_cmd(const Cmd::T &cmd);

	void    program_flash(std::span<const uint8_t> data);
	uint8_t read_flash_byte(const uint16_t addr);

	void write_flash_page(const uint16_t page_addr, std::span<const uint8_t> data);
	void wait_for_ready();

private:
	static spi_inst_t *const spi_intf;
	// static constexpr uint32_t baudrate = 2000000;
	static constexpr uint32_t baudrate = 50000;

	// TODO: This is different for attiny parts
	// Page size in (16 bit) words
	static constexpr uint16_t flash_page_words = 64;
	static constexpr auto     flash_page_bytes = 2 * flash_page_words;

	static constexpr auto sclk_pin   = Pins::spi_sclk;
	static constexpr auto rx_pin     = Pins::spi_rx;
	static constexpr auto tx_pin     = Pins::spi_tx;
	static constexpr auto rst_n_pin  = Pins::rst_n;
	static constexpr auto pwr_en_pin = Pins::pwr_en;
};

} // namespace TargetInterfaces
