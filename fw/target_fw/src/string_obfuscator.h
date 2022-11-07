#include <stddef.h>
#include <stdint.h>

// Helpers to allow lightweight compile time encryption and runtime decryption
// of strings for obfuscation purposes. The string is XORed with values from
// an XorShift32 PRNG. obfuscate and deobfuscate should be used
// to encrypt and decrypt the string respectively
//
// e.g.
// auto flag = obfuscate<0xDEADBEEF>("flag{...}");
// // Outputs encrypted string
// puts(flag.data());
//
// flag.deobfuscate();
// // Outputs decrypted string
// puts(flag.data());
//
// It Seems to work on AVR but may not work there or anywhere else. It's also
// not the tidiest. Oh well. Use a high optimisation level and grep through the
// strings output on the binary

namespace StringObfuscator {

constexpr void encode_string(
    const uint32_t seed, const char plain[], char cipher[], const size_t len) {
	// XorShift32
	uint32_t state = seed;
	for (size_t i = 0; i < len; i++) {
		state ^= state << 13;
		state ^= state >> 17;
		state ^= state << 5;
		cipher[i] = plain[i] ^ static_cast<char>(state);
	}
}

template <uint32_t SEED, size_t N>
class String {
public:
	constexpr String(const char (&plain_text)[N]) : string_data{} {
		encode_string(SEED, plain_text, string_data, N);
	}
	constexpr size_t size() const { return N; }
	constexpr const char* data() const { return string_data; }

	void deobfuscate() { encode_string(SEED, string_data, string_data, N); }

private:
	char string_data[N];
};

// Helper constructor that automatically determines N
template <uint32_t SEED, size_t N>
constexpr String<SEED, N> obfuscate(const char (&plain)[N]) {
	return String<SEED, N>{plain};
}

} // namespace StringObfuscator
