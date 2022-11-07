// The target controller exposes a basic interface from some of the other
// interfaces to do with controlling the power/rst/clock lines of the target
// as well as programming it

#pragma once
#include <cstdint>
#include <span>
#include "clk_controller.h"
#include "pins.h"
#include "serial.h"

namespace TargetInterfaces {

class TargetController {
public:
	TargetController();
	// TODO: Destructor

	void program_flash(std::span<const uint8_t> data);
	// Returns true only if all bytes match for length of expected
	bool verify_flash(std::span<const uint8_t> expected);

	struct AVRFuses {
		uint8_t low;
		uint8_t high;
		uint8_t extended;
	};
	void write_fuses(const AVRFuses fuses);
	AVRFuses read_fuses();

	void enable_pwr();
	void disable_pwr();
	void reset();

	void run();

private:
	ClkController clk;
	Serial serial;

	static constexpr auto pwr_en_pin = Pins::pwr_en;
	static constexpr auto rst_n_pin = Pins::rst_n;
};

} // namespace TargetInterfaces
