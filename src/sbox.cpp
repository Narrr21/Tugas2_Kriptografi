#if __has_include("sbox.hpp")
#include "sbox.hpp"
#else
#include "../include/sbox.hpp"
#endif

#include <utility>

namespace kripto {

std::array<std::uint8_t, 256> generateSBox(const std::array<std::uint8_t, 16>& keyBytes) {
	std::array<std::uint8_t, 256> sbox{};
	for (int i = 0; i < 256; ++i) {
		sbox[i] = static_cast<std::uint8_t>(i);
	}
	std::uint8_t j = 0;
	for (int i = 0; i < 256; ++i) {
		j = static_cast<std::uint8_t>(j + sbox[i] + keyBytes[i % 16] + 0x5A);
		std::swap(sbox[i], sbox[j]);
	}
	return sbox;
}

std::uint64_t substituteBytes(std::uint64_t x, const std::array<std::uint8_t, 256>& sbox) {
	std::uint64_t sub = 0;
	for (int i = 0; i < 8; ++i) {
		std::uint8_t b = static_cast<std::uint8_t>(x >> (i * 8));
		sub |= static_cast<std::uint64_t>(sbox[b]) << (i * 8);
	}
	return sub;
}

} // namespace kripto
