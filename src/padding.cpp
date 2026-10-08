#if __has_include("padding.hpp")
#include "padding.hpp"
#else
#include "../include/padding.hpp"
#endif

#include <stdexcept>

namespace kripto {

std::vector<std::uint8_t> padPKCS7(const std::vector<std::uint8_t>& data) {
	std::size_t padLen = 16 - (data.size() % 16);
	std::vector<std::uint8_t> padded = data;
	padded.insert(padded.end(), padLen, static_cast<std::uint8_t>(padLen));
	return padded;
}

std::vector<std::uint8_t> unpadPKCS7(const std::vector<std::uint8_t>& data) {
	if (data.empty() || (data.size() % 16) != 0) {
		throw std::runtime_error("Invalid PKCS#7 padded block alignment");
	}
	std::uint8_t padLen = data.back();
	if (padLen == 0 || padLen > 16 || padLen > data.size()) {
		throw std::runtime_error("Invalid PKCS#7 padding length");
	}

	// for (std::size_t i = data.size() - padLen; i < data.size(); ++i) {
	// 	if (data[i] != padLen) {
	// 		throw std::runtime_error("Corrupted PKCS#7 padding bytes");
	// 	}
	// }

	std::uint8_t diff = 0;
	for (std::size_t i = data.size() - padLen; i < data.size(); ++i) {
		diff |= (data[i] ^ padLen);
	}

	if (diff != 0) {
		throw std::runtime_error("Corrupted PKCS#7 padding bytes");
	}
	return std::vector<std::uint8_t>(data.begin(), data.end() - padLen);
}

} // namespace kripto
