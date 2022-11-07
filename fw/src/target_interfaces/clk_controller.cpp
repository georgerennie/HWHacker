#include "clk_controller.h"
#include <hardware/irq.h>
#include <pico/stdlib.h>
#include <algorithm>
#include <pio_builder.hpp>

// TODO: A lot of this could do with refactoring for safety/clarity

namespace TargetInterfaces {

ClkController* ClkController::self_instance = nullptr;
const PIO      ClkController::pio_inst      = pio0;

struct ClockProgram : public PIOBuilder::Program<1> {
	// Non glitching constructor
	constexpr ClockProgram(const uint8_t half_period) {
		wrap_target();
		nop().side(1)[half_period - 1];
		nop().side(0)[half_period - 1];
		wrap();
	}

	// Glitching constructor
	constexpr ClockProgram(
	    const uint8_t half_period, const uint8_t on_period, const uint8_t off_period
	) {
		wrap_target();
		// clang-format off
		const auto glitch = label();
			out_x(1)          .side(0) [off_period - 1];
			jmp_not_x(glitch) .side(1) [on_period - 1];
			nop()             .side(1) [half_period - 1 - on_period];
			nop()             .side(0) [half_period - 1 - off_period];
		// clang-format on
		wrap();
	}
};

ClkController::ClkController() {
	if (self_instance) {
		panic("Instance of ClkController already exists. Only one can exist at once");
	}
	self_instance = this;

	pio_gpio_init(pio_inst, clk_pin);
	pio_sm = pio_claim_unused_sm(pio_inst, true);
	pio_sm_set_consecutive_pindirs(pio_inst, pio_sm, clk_pin, 1, true);

	for (auto& chan : dma_chans) {
		chan.num = dma_claim_unused_channel(true);
	}
}

ClkController::~ClkController() {
	stop();

	pio_sm_unclaim(pio_inst, pio_sm);
	for (const auto chan : dma_chans) {
		dma_channel_unclaim(chan.num);
	}
	self_instance = nullptr;
}

void ClkController::run_normal() {
	if (running) {
		stop();
	}
	running = true;

	ClockProgram program(16);
	pio_prog_offset = pio_add_program(pio_inst, program);
	pio_prog_len    = program.size();

	auto config = program.get_default_config(pio_prog_offset);
	sm_config_set_sideset_pins(&config, clk_pin);
	sm_config_set_clkdiv_int_frac(&config, 1, 0);
	pio_sm_init(pio_inst, pio_sm, pio_prog_offset, &config);
	pio_sm_set_enabled(pio_inst, pio_sm, true);
}

void ClkController::run_glitches(std::span<ClkController::GlitchRegion> regions) {
	if (running) {
		stop();
	}
	running = true;

	// Setup SM program
	ClockProgram program(16, 4, 5);
	pio_prog_offset = pio_add_program(pio_inst, program);
	pio_prog_len    = program.size();

	auto config = program.get_default_config(pio_prog_offset);
	sm_config_set_sideset_pins(&config, clk_pin);
	sm_config_set_clkdiv_int_frac(&config, 1, 0);
	sm_config_set_out_shift(&config, false, true, 32);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);
	pio_sm_init(pio_inst, pio_sm, pio_prog_offset, &config);

	for (size_t i = 0; i < dma_chans.size(); i++) {
		auto& chan  = dma_chans[i];
		chan.config = dma_channel_get_default_config(chan.num);
		channel_config_set_read_increment(&chan.config, true);
		channel_config_set_write_increment(&chan.config, false);
		channel_config_set_transfer_data_size(&chan.config, DMA_SIZE_32);
		channel_config_set_dreq(&chan.config, pio_get_dreq(pio_inst, pio_sm, true));

		const auto next_chan = (i + 1) % dma_chans.size();
		channel_config_set_chain_to(&chan.config, dma_chans[next_chan].num);
		dma_channel_set_irq0_enabled(chan.num, true);
	}

	irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
	irq_set_enabled(DMA_IRQ_0, true);

	current_region = regions.begin();
	end_region     = regions.end();
	glitch_idx     = 0;

	// Configure buffers ready ahead of transfer
	for (const auto& chan : dma_chans) {
		prepare_next_dma_buf();
	}

	pio_sm_clear_fifos(pio_inst, pio_sm);
	dma_channel_start(dma_chans[dma_chan_idx].num);
	pio_sm_set_enabled(pio_inst, pio_sm, true);
}

void ClkController::stop() {
	pio_sm_set_enabled(pio_inst, pio_sm, false);

	const pio_program_t dummy_prog = {.length = pio_prog_len};
	pio_remove_program(pio_inst, &dummy_prog, pio_prog_offset);

	for (auto& chan : dma_chans) {
		dma_channel_abort(chan.num);
		dma_channel_set_irq0_enabled(chan.num, false);
	}

	running = false;
}

void ClkController::prepare_next_dma_buf() {
	auto& chan = dma_chans[dma_chan_idx];
	auto& buf  = chan.buf;
	dma_channel_configure(
	    chan.num, &chan.config, &pio_inst->txf[pio_sm], buf.data(), buf.size(), false
	);

	dma_chan_idx = (dma_chan_idx + 1) % dma_chans.size();

	size_t buf_idx = 0;
	while (buf_idx < buf.size()) {
		// If past end, just write non glitches
		if (current_region == end_region) {
			std::fill(&buf[buf_idx], buf.end(), 0xFFFFFFFF);
			return;
		}

		const auto bits_left  = current_region->first - glitch_idx;
		const auto words_left = bits_left / 32;

		const auto word_fill_val = current_region->second ? 0 : 0xFFFFFFFF;

		// If the region overflows this buffer
		if (buf_idx + words_left >= dma_buf_size) {
			const auto num_words = dma_buf_size - buf_idx;
			std::fill_n(&buf[buf_idx], num_words, word_fill_val);
			glitch_idx += num_words * 32;
			return;
		}

		// This region ends before the end of the buffer
		else {
			// Fill up to word that has changes in
			std::fill_n(&buf[buf_idx], words_left, word_fill_val);
			glitch_idx += words_left * 32;
			buf_idx += words_left;

			// Iterate through bits in word
			buf[buf_idx] = 0;
			for (size_t i = 0; i < 32; i++) {
				// If reached end of regions, just write non glitch cycles
				if (current_region == end_region) {
					buf[buf_idx] |= 0xFFFFFFFF >> i;
					break;
				}
				// If still in glitch region write bit if appropriate
				else if (glitch_idx < current_region->first) {
					if (!current_region->second) {
						buf[buf_idx] |= 1 << (31 - i);
					}
					glitch_idx++;
				}
				// If on region boundary restart as new region
				else {
					current_region++;
					glitch_idx = 0;
					i--;
				}
			}
			buf_idx++;
		}
	}
}

void ClkController::dma_handler() {
	// TODO: Check source is definitely from the channels this uses
	self_instance->prepare_next_dma_buf();
	// Just clear all dma interrupts
	for (const auto& chan : self_instance->dma_chans) {
		dma_hw->ints0 = 1 << chan.num;
	}
}

} // namespace TargetInterfaces
