#if __has_include("operations.hpp")
#include "operations.hpp"
#else
#include "../include/operations.hpp"
#endif

#include <bit>

namespace kripto {

Block128 bytesToBlock(const std::uint8_t* src) {
	Block128 b{};
	for (int i = 0; i < 8; ++i) {
		b.hi = (b.hi << 8) | src[i];
	}
	for (int i = 8; i < 16; ++i) {
		b.lo = (b.lo << 8) | src[i];
	}
	return b;
}

void blockToBytes(Block128 b, std::uint8_t* dst) {
	for (int i = 7; i >= 0; --i) {
		dst[i] = static_cast<std::uint8_t>(b.hi & 0xFF);
		b.hi >>= 8;
	}
	for (int i = 15; i >= 8; --i) {
		dst[i] = static_cast<std::uint8_t>(b.lo & 0xFF);
		b.lo >>= 8;
	}
}

std::uint64_t rotl64(std::uint64_t val, int shift) {
	return std::rotl(val, shift);
}

std::uint64_t rotr64(std::uint64_t val, int shift) {
	return std::rotr(val, shift);
}

std::uint64_t addMod64(std::uint64_t a, std::uint64_t b) {
	return a + b;
}

std::uint64_t bitTransposition(std::uint64_t x) {
	return std::rotl(x, 17) ^ std::rotr(x, 29);
}

std::uint64_t diffuse(std::uint64_t sub, std::uint64_t roundKey) {
	sub = std::rotl(sub, 31);
	sub += roundKey;
	sub ^= std::rotr(sub, 13);
	return sub;
}

unsigned hammingDistance(Block128 left, Block128 right) {
	return static_cast<unsigned>(std::popcount(left.hi ^ right.hi) + std::popcount(left.lo ^ right.lo));
}

} // namespace kripto
