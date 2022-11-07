// Bit banged SPI controller that only transmits data. CPOL = 0, CPHA = 0, CS
// is active low, MSB first 8 bit transmissions

#pragma once
#include "io_pin.h"
#include "pins.h"

class BBSPI {
public:
	void putchar(char c);

private:
	IOPin<Pins::SPI_SCLK, true, 0> sclk;
	IOPin<Pins::SPI_CS_N, true, 1> cs_n;
	IOPin<Pins::SPI_TX, true, 0>   tx;
};
