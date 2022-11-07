#include <pico/stdlib.h>
#include <stdio.h>
#include "rp2040/hardware.h"
#include "target_fw.h"
#include "target_interfaces/pins.h"
#include "target_interfaces/target_controller.h"

int main() {
	RP2040::Hardware::init_all();

	// Clear Screen
	printf("\x1B[2J");

	TargetInterfaces::TargetController target;

	printf("Writing %u bytes...\n", target_fw_bin_len);
	target.program_flash(target_fw_bin);
	printf("Wrote %u bytes...\n", target_fw_bin_len);
	printf("Verification passed: %s\n", target.verify_flash(target_fw_bin) ? "yes" : "no");
	// target.write_fuses({.low = 0xD0, .high = 0xD9, .extended = 0xFF});
	// const auto fuses = target.read_fuses();
	// printf("Fuses (l, h, e) %X, %X, %X\n", fuses.low, fuses.high, fuses.extended);

	// TODO: FIX ISSUES WITH SPI PROG and SERIAL using same SPI port and not
	// clearing up after themselves properly (Serial should exist in
	// TargetController, not here...)
	TargetInterfaces::Serial s;
	target.run();
	
	// std::pair<uint32_t, bool> glitches[] = {
	//     {10022005, false},
	//     {10, true},
	// };
	// clk.run_glitches(glitches);
	// sleep_ms(5000);
	// clk.stop();

	for (;;) {}
}
