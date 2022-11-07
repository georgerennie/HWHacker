// Big enumeration of the pins for different items
#pragma once
#include <cinttypes>

namespace TargetInterfaces::Pins {
inline constexpr uint32_t pwr_en = 10;
inline constexpr uint32_t rst_n = 22;
inline constexpr uint32_t clk = 21;

inline constexpr uint32_t spi_cs_n = 5;
inline constexpr uint32_t spi_sclk = 6;
inline constexpr uint32_t spi_rx = 16;
inline constexpr uint32_t spi_tx = 19;
} // namespace TargetInterfaces::Pins
