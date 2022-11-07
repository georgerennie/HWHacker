#pragma once
#include <inttypes.h>
#include <stddef.h>
#include "bit_bang_spi.h"

class Serial {
public:
	inline void putchar(const char c) { spi.putchar(c); }

	void puts(const char* s);

	void endl();
	void space();

	void        write(const char* s);
	inline void write(char* s) { write(const_cast<const char*>(s)); }

	void        write(const char* s, const size_t len);
	inline void write(char* s, const size_t len) { write(const_cast<const char*>(s), len); }

	inline void write(uint8_t val) { write(static_cast<uint32_t>(val)); }
	inline void write(uint16_t val) { write(static_cast<uint32_t>(val)); }
	void        write(uint32_t val);

	void write_hex(uint8_t val, const bool leader = false);
	void write_hex(const uint16_t val, const bool leader = false);
	void write_hex(const uint32_t val, const bool leader = false);

private:
	BBSPI spi;
};
