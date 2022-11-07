#pragma once

#include "inttypes.h"

namespace Maths {

template <typename Q, typename R>
struct DivModResult {
	Q quot;
	R rem;
};

// Very compact iterative divmod. Keeps subtracting n until underflow, so takes
// O(a / n) time
constexpr DivModResult<uint32_t, uint32_t> iterative_division(uint32_t a, const uint32_t n) {
	for (uint32_t quot = 0;; quot++) {
		const auto last_a = a;
		a -= n;

		const auto top_a      = static_cast<uint8_t>(a >> 24);
		const auto top_last_a = static_cast<uint8_t>(last_a >> 24);
		if (top_a > top_last_a)
			return {quot, last_a};
	}
}

constexpr DivModResult<uint32_t, uint32_t> intrinsic_division(const uint32_t a, const uint32_t n) {
	return {a / n, a % n};
}

// Division by 10. Only confirmed to be accurate for powers of 10 greater than
// or equal to 100.
constexpr uint32_t approx_div_10(uint32_t val) {
	val += val >> 1;
	val = (val >> 4) + (val >> 8);
	val += val >> 8;
	val += val >> 16;

	uint8_t offset = (val & 1) ? 1 : 2;

	return val + offset;
}

} // namespace Maths
