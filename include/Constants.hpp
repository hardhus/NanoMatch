#pragma once
#include <cstddef>
#include <cstdint>

namespace NanoMatch {
constexpr size_t MAX_ORDERS = 100'000;

constexpr uint64_t MIN_PRICE = 40'000;
constexpr uint64_t TICK_SIZE = 1;

constexpr uint64_t MAX_PRICE_TICKS = 100'000;

constexpr size_t NUM_BIT_WORDS = (MAX_PRICE_TICKS + 63) / 64;
} // namespace NanoMatch
