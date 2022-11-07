#include "serial.h"
#include "maths_utils.h"

void Serial::puts(const char* s) {
	write(s);
	endl();
}

void Serial::endl() { putchar('\n'); }
void Serial::space() { putchar(' '); }

void Serial::write(const char* s) {
	for (size_t i = 0; s[i] != '\0'; i++) {
		putchar(s[i]);
	}
}

void Serial::write(const char* s, const size_t len) {
	for (size_t i = 0; i < len; i++) {
		putchar(s[i]);
	}
}

void Serial::write(uint32_t val) {
	// 2^32 - 1 uses 10 digits
	// constexpr auto digits = 10;
	// char buf[digits];

	// uint8_t i;
	// for (i = digits - 1;; i--) {
	//     // This iterative div is gonna be very very slow, but it is compact
	//     // Use it or algorithm below if running out of space, otherwise use
	//     // intrinsic div
	//     // const auto divmod = Maths::iterative_division(val, 10);
	//     const auto divmod = Maths::intrinsic_division(val, 10);
	//     buf[i] = static_cast<char>(divmod.rem) + '0';
	//     val = divmod.quot;
	//     if (val == 0)
	//         break;
	// }

	// write(&buf[i], digits - i);

	// 10 digits total, but the smallest digit is special cased
	constexpr auto digits  = 9;
	uint32_t       divider = 1000000000;
	bool           visible = false;

	for (uint8_t i = 0; i < digits; i++) {
		const auto divmod = Maths::iterative_division(val, divider);
		divider           = Maths::approx_div_10(divider);
		val               = divmod.rem;

		const auto quot = static_cast<char>(divmod.quot);
		if (quot != 0 || visible) {
			putchar(quot + '0');
			visible = true;
		}
	}
	putchar(static_cast<char>(val) + '0');
}

void Serial::write_hex(uint8_t val, const bool leader) {
	if (leader) {
		putchar('0');
		putchar('x');
	}

	for (uint8_t i = 0; i < 2; i++) {
		const uint8_t nibble = val >> 4;
		putchar(static_cast<char>(nibble + (nibble < 10 ? '0' : 'A' - 10)));
		val <<= 4;
	}
}

void Serial::write_hex(const uint16_t val, const bool leader) {
	write_hex(static_cast<uint8_t>(val >> 8), leader);
	write_hex(static_cast<uint8_t>(val), false);
}

void Serial::write_hex(const uint32_t val, const bool leader) {
	write_hex(static_cast<uint16_t>(val >> 16), leader);
	write_hex(static_cast<uint16_t>(val), false);
}
