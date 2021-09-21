// A minimal implementation of the ATtiny SPI programming protocol needed for
// programming the target AVR chip

#pragma once

#include <hardware/spi.h>
#include <array>
#include <cinttypes>

namespace TargetInterfaces {

class SPIProg {
public:
	SPIProg() { init(); }
	static void init();

	static void start_programming();
	static void restart_programming();
	static void finish_programming();

	static void write_flash_program(const uint8_t data[], uint16_t length, bool erase = false);

private:
	struct Cmd {
		using T = std::array<uint8_t, 4>;
		static constexpr T ProgEn = {0xAC, 0x53, 0x00, 0x00};
		static constexpr T ChipErase = {0xAC, 0x80, 0x00, 0x00};
		static constexpr T PollReadyBusy = {0xF0, 0x00, 0x00, 0x00};

		static constexpr T LoadPGMPageHigh(uint16_t word, uint8_t data) {
			return {0x48, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), data};
		}

		static constexpr T LoadPGMPageLow(uint16_t word, uint8_t data) {
			return {0x40, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), data};
		}

		static constexpr T WritePGMPage(uint16_t page) {
			return {0x4C, static_cast<uint8_t>(page >> 8), static_cast<uint8_t>(page), 0x00};
		}

		static constexpr T ReadPGMPageHigh(uint16_t word) {
			return {0x28, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), 0x00};
		}
		static constexpr T ReadPGMPageLow(uint16_t word) {
			return {0x20, static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word), 0x00};
		}
		static constexpr T ReadLFuse = {0x50, 0x00, 0x00, 0x00};
		static constexpr T ReadHFuse = {0x58, 0x08, 0x00, 0x00};
		static constexpr T ReadEFuse = {0x50, 0x08, 0x00, 0x00};

		static constexpr T WriteLFuse(uint8_t val) { return {0xAC, 0xA0, 0x00, val}; };
		static constexpr T WriteHFuse(uint8_t val) { return {0xAC, 0xA8, 0x00, val}; };
		static constexpr T WriteEFuse(uint8_t val) { return {0xAC, 0xA4, 0x00, val}; };
	};

	static void write_flash_page(const uint16_t page_addr, const uint8_t data[], uint16_t len);
	static Cmd::T transfer_cmd(const Cmd::T &cmd);
	static void wait_for_ready();

	static spi_inst_t *const spi_intf;
	static constexpr uint32_t baudrate = 1000000;

	// TODO: This is different for attiny parts
	// Page size in (16 bit) words
	static constexpr uint16_t flash_page_size = 128;

	static constexpr uint32_t sclk_pin = 6;
	static constexpr uint32_t rx_pin = 16;
	static constexpr uint32_t tx_pin = 19;
	static constexpr uint32_t rst_n_pin = 22;
	static constexpr uint32_t pwr_en_pin = 10;
};

} // namespace TargetInterfaces
