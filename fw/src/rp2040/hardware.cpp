#include "hardware.h"
#include <hardware/clocks.h>
#include <hardware/vreg.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>

namespace RP2040::Hardware {

static void init_stdio() { stdio_init_all(); }

static void init_clock_freq() {
	// Make processor go nyooooom

	// This needs to be an achievable multiple. 250, 300, 351, 400, 412, 416 MHz
	// all work.
	// Use src/rp2_common/hardware_clocks/scripts/vcocalc.py to calculate valid
	// freqs.
	// If you use an invalid freq, it will just default to 125MHz
	// Note above about 250MHz you need to set PICO_FLASH_SPI_CLKDIV to 4 for the
	// boot stage 2 compilation.
	// This can be done with
	// target_compile_definitions(bs2_default PRIVATE PICO_FLASH_SPI_CLKDIV=4)
	// in CMakeLists.txt

	constexpr auto freq = 400 * MHZ;

	// These are rough and might not work for all freqs
	if constexpr (freq >= 400 * MHZ) {
		vreg_set_voltage(VREG_VOLTAGE_1_30);
	} else if constexpr (freq >= 350 * MHZ) {
		vreg_set_voltage(VREG_VOLTAGE_1_25);
	} else if constexpr (freq >= 300 * MHZ) {
		vreg_set_voltage(VREG_VOLTAGE_1_15);
	} else if constexpr (freq >= 250 * MHZ) {
		vreg_set_voltage(VREG_VOLTAGE_1_10);
	}

	set_sys_clock_khz(freq / KHZ, false);
	clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, freq, freq);
}

void init_all() {
	init_clock_freq();

	init_stdio();
}

} // namespace RP2040::Hardware
