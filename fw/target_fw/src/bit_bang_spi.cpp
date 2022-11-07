#include "bit_bang_spi.h"

void BBSPI::putchar(char c) {
	cs_n.clear();

	for (uint8_t i = 0; i < 8; i++) {
		tx.write(c & 0x80);
		sclk.set();
		c <<= 1U;
		sclk.clear();
	}

	cs_n.set();
}
