#pragma once

#include <hardware/dma.h>
#include <hardware/pio.h>
#include <array>
#include <pio_builder.hpp>

namespace TargetInterfaces {

class ClkController {
public:
	ClkController() { init(); }
	static void init();

	static void start(const uint32_t data[], const size_t words);

private:
	static auto generate_pio_program(const uint8_t half_period);

	static PIO pio_inst;
	static constexpr uint16_t pio_sm = 0;
	static constexpr uint16_t dma_chan = 0;
	static dma_channel_config dma_config;

	static constexpr uint32_t clk_pin = 21;
};

} // namespace TargetInterfaces
