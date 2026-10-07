#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace kripto {

// Key-dependent S-box generation (Fisher-Yates permutation keyed with master key bytes)
std::array<std::uint8_t, 256> generateSBox(const std::array<std::uint8_t, 16>& keyBytes);

// Key-dependent S-box byte substitution on a 64-bit word
std::uint64_t substituteBytes(std::uint64_t x, const std::array<std::uint8_t, 256>& sbox);

} // namespace kripto
