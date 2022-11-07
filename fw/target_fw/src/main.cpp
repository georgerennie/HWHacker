#include <avr/io.h>
#include "serial.h"
#include "string_obfuscator.h"

int main() {
	Serial serial;

	for (;;) {
		volatile uint32_t cnt = 0;
		for (uint8_t i = 0; i < 4; i++) {
			for (uint8_t j = 0; j < 250; j++) {
				for (uint8_t k = 0; k < 250; k++) {
					cnt++;
				}
			}
		}

		serial.write("cnt:");
		// serial.write(cnt);
		serial.write(0xFFFFFFFF);
		serial.endl();
	}

	// TODO: Put this in progmem and adapt obfuscation to load string from
	// progmem char by char and print, reducing the size of intermediate buffers
	// and thus improving memory usage
	// auto flag = StringObfuscator::obfuscate<0xDEADBEEF>("flag{...}");

	// Outputs encrypted string
	// serial.puts(flag.data());

	// flag.deobfuscate();
	// Outputs decrypted string
	// serial.puts(flag.data());
}
