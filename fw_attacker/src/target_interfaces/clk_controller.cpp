#include "clk_controller.h"
#include <pico/stdlib.h>

namespace TargetInterfaces {

PIO ClkController::pio_inst = pio0;
dma_channel_config ClkController::dma_config;

auto ClkController::generate_pio_program(const uint8_t half_period) {
	using namespace PIOBuilder;
	Program<4, 1> p;

	p.wrap_target();
	const auto glitch_lbl = p.out(OutDest::X, 1, 0, 0);
	p.jmp(JmpCond::X_Zero, glitch_lbl, 0, 1);

	p.nop(half_period - 2, 1);
	p.nop(half_period - 2, 0);
	p.wrap();

	return p;
}

void ClkController::init() {
	const auto program = generate_pio_program(15);
	const auto offset = pio_add_program(pio_inst, program);

	pio_gpio_init(pio_inst, clk_pin);
	pio_sm_set_consecutive_pindirs(pio_inst, pio_sm, clk_pin, 1, true);

	auto config = program.get_default_config(offset);
	sm_config_set_sideset_pins(&config, clk_pin);
	sm_config_set_clkdiv_int_frac(&config, 100, 0);
	sm_config_set_out_shift(&config, false, true, 32);
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);

	dma_config = dma_channel_get_default_config(dma_chan);
	channel_config_set_read_increment(&dma_config, true);
	channel_config_set_write_increment(&dma_config, false);
	channel_config_set_dreq(&dma_config, pio_get_dreq(pio_inst, pio_sm, true));

	pio_sm_init(pio_inst, pio_sm, offset, &config);
}

void ClkController::start(const uint32_t data[], const size_t words) {
	pio_sm_clear_fifos(pio_inst, pio_sm);
	dma_channel_configure(dma_chan, &dma_config, &pio_inst->txf[pio_sm], data, words, true);
	pio_sm_set_enabled(pio_inst, pio_sm, true);
}

} // namespace TargetInterfaces
