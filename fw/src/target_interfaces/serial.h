// An interface to the serial output from the target

#pragma once

#include <hardware/spi.h>
#include "pins.h"

namespace TargetInterfaces {

class Serial {
public:
	Serial() { init(); }
	~Serial() { deinit(); }
	void init();
	void deinit();

private:
	static void interrupt_handler();

	static spi_inst_t *const spi_intf;

	static constexpr auto sclk_pin = Pins::spi_sclk;
	static constexpr auto cs_n_pin = Pins::spi_cs_n;
	static constexpr auto rx_pin = Pins::spi_rx;
	static constexpr auto tx_pin = Pins::spi_tx;
};

} // namespace TargetInterfaces
