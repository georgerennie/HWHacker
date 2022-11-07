#include "target_controller.h"
#include <pico/time.h>
#include "spi_prog.h"

namespace TargetInterfaces {

TargetController::TargetController() {
	gpio_init(rst_n_pin);
	gpio_set_dir(rst_n_pin, GPIO_OUT);
	gpio_put(pwr_en_pin, 1);

	gpio_init(pwr_en_pin);
	gpio_set_dir(pwr_en_pin, GPIO_OUT);
	gpio_put(pwr_en_pin, 0);
}

// TODO: Tidy the mass of boilerplate in every program command

void TargetController::program_flash(std::span<const uint8_t> data) {
	serial.deinit();
	clk.run_normal();
	SPIProg prog;
	// TODO: start and finish_programming could be moved to SPIProg constructor/
	// destructor
	prog.start_programming();
	// TODO: Verify programming is correct and add timeout
	prog.program_flash(data);
	prog.finish_programming();
	sleep_ms(10);
	clk.stop();
	serial.init();
}

bool TargetController::verify_flash(std::span<const uint8_t> expected) {
	serial.deinit();
	clk.run_normal();
	SPIProg prog;
	prog.start_programming();

	bool is_valid = true;
	for (size_t i = 0; i < expected.size_bytes(); i++) {
		if (prog.read_flash_byte(i) != expected[i]) {
			is_valid = false;
			break;
		}
	}

	prog.finish_programming();
	sleep_ms(10);
	clk.stop();
	serial.init();
	return is_valid;
}

TargetController::AVRFuses TargetController::read_fuses() {
	serial.deinit();
	clk.run_normal();
	SPIProg prog;
	prog.start_programming();

	AVRFuses fuses;
	fuses.low      = prog.execute_cmd(SPIProg::Cmd::ReadLFuse);
	fuses.high     = prog.execute_cmd(SPIProg::Cmd::ReadHFuse);
	fuses.extended = prog.execute_cmd(SPIProg::Cmd::ReadEFuse);

	prog.finish_programming();
	clk.stop();
	serial.init();

	return fuses;
}

void TargetController::write_fuses(const AVRFuses fuses) {
	serial.deinit();
	clk.run_normal();
	SPIProg prog;
	prog.start_programming();

	prog.execute_cmd(SPIProg::Cmd::WriteLFuse(fuses.low));
	prog.execute_cmd(SPIProg::Cmd::WriteLFuse(fuses.high));
	prog.execute_cmd(SPIProg::Cmd::WriteLFuse(fuses.extended));

	prog.finish_programming();
	clk.stop();
	serial.init();
}

void TargetController::enable_pwr() {
	gpio_put(rst_n_pin, 1);
	gpio_put(pwr_en_pin, 1);
}

void TargetController::disable_pwr() { gpio_put(pwr_en_pin, 0); }

void TargetController::reset() {
	disable_pwr();
	sleep_ms(10);
	enable_pwr();
}

void TargetController::run() {
	enable_pwr();
	clk.run_normal();
}

} // namespace TargetInterfaces
