#pragma once

#include <cstddef>
#include <cstdint>

namespace kripto {

// 128-bit block structure
struct Block128 {
	std::uint64_t hi{0};
	std::uint64_t lo{0};

	bool operator==(const Block128& other) const = default;
};

// Conversions
Block128 bytesToBlock(const std::uint8_t* src);
void blockToBytes(Block128 b, std::uint8_t* dst);

// Bitwise rotations (std::rotl / std::rotr)
std::uint64_t rotl64(std::uint64_t val, int shift);
std::uint64_t rotr64(std::uint64_t val, int shift);

// Modulo 2^64 addition
std::uint64_t addMod64(std::uint64_t a, std::uint64_t b);

// P-box and permutation / transposition helpers
std::uint64_t bitTransposition(std::uint64_t x);
std::uint64_t diffuse(std::uint64_t sub, std::uint64_t roundKey);

// Analysis Utilities
unsigned hammingDistance(Block128 left, Block128 right);

} // namespace kripto
