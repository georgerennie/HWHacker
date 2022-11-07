#pragma once

#include <hardware/dma.h>
#include <hardware/pio.h>
#include <array>
#include <span>
#include <utility>
#include "pins.h"

namespace TargetInterfaces {

class ClkController {
public:
	// SAFETY: Panics if more than one instance of ClkController is constructed
	ClkController();
	ClkController(const ClkController&) = delete;
	~ClkController();

	void run_normal();

	// TODO: Is this the right way of specifying glitch periods
	// Run the clock with glitches specified by regions
	// Each item in regions specifies the length of a region, and the bool
	// determines whether that region is glitched or not
	// SAFETY: The memory pointed to by the span must remain valid until the
	// glitching has finished
	using GlitchRegion = std::pair<uint32_t, bool>;
	void run_glitches(std::span<GlitchRegion> regions);

	void stop();

private:
	auto glitch_program(
	    const uint8_t half_period, const uint8_t on_period, const uint8_t off_period
	);

	auto no_glitch_program(const uint8_t half_period);

	void        prepare_next_dma_buf();
	static void dma_handler();

	static ClkController* self_instance;

	// Buffer size in 32 bit words
	static constexpr auto dma_buf_size = 128U;
	// Number of dma channels to use for round robin
	static constexpr auto num_dma_chans = 2U;

	// TODO: Wrap the buffer filling functions etc into a class for a
	// transaction
	struct DmaChanData {
		std::array<uint32_t, dma_buf_size> buf;
		uint16_t                           num;
		dma_channel_config                 config;
	};
	std::array<DmaChanData, num_dma_chans> dma_chans;
	size_t                                 dma_chan_idx = 0;

	std::span<GlitchRegion>::iterator current_region, end_region;
	// Current number of bits through region
	size_t glitch_idx = 0;

	bool             running = false;
	static const PIO pio_inst;
	uint16_t         pio_sm;
	uint8_t          pio_prog_offset, pio_prog_len;

	static constexpr auto clk_pin = Pins::clk;
};

} // namespace TargetInterfaces
