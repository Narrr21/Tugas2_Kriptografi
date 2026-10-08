#pragma once

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <vector>
#include "../src/my_cipher.cpp"

namespace test_support {

inline const kripto::Block128 key{
	0x0123456789ABCDEFULL,
	0xFEDCBA9876543210ULL
};

inline const kripto::Block128 iv{
	0xA5A5A5A5A5A5A5A5ULL,
	0x5A5A5A5A5A5A5A5AULL
};

inline std::vector<std::uint8_t> makeData(std::size_t size, std::uint64_t seed = 0xC0FFEEULL) {
	std::mt19937_64 generator(seed);
	std::vector<std::uint8_t> data(size);
	for (std::uint8_t& value : data) {
		value = static_cast<std::uint8_t>(generator() & 0xFFU);
	}
	return data;
}

inline unsigned hammingDistance(
	const std::vector<std::uint8_t>& left,
	const std::vector<std::uint8_t>& right
) {
	unsigned distance = 0;
	const std::size_t size = std::min(left.size(), right.size());
	for (std::size_t i = 0; i < size; ++i) {
		distance += static_cast<unsigned>(
			std::popcount(static_cast<unsigned>(left[i] ^ right[i]))
		);
	}
	return distance + static_cast<unsigned>(8 * (left.size() > right.size()
		? left.size() - right.size()
		: right.size() - left.size()));
}

inline std::string modeName(kripto::CipherMode mode) {
	switch (mode) {
		case kripto::CipherMode::ECB: return "ECB";
		case kripto::CipherMode::CBC: return "CBC";
		case kripto::CipherMode::CFB: return "CFB";
		case kripto::CipherMode::OFB: return "OFB";
		case kripto::CipherMode::CTR: return "CTR";
	}
	return "UNKNOWN";
}

inline std::vector<kripto::CipherMode> allModes() {
	return {
		kripto::CipherMode::ECB,
		kripto::CipherMode::CBC,
		kripto::CipherMode::CFB,
		kripto::CipherMode::OFB,
		kripto::CipherMode::CTR
	};
}

}
