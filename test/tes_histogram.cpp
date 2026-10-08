#include <array>
#include <iomanip>
#include <iostream>
#include <vector>
#include "test_helpers.hpp"

std::size_t distinctByteCount(const std::vector<std::uint8_t>& data) {
	std::array<bool, 256> present{};
	for (std::uint8_t value : data) {
		present[value] = true;
	}
	std::size_t count = 0;
	for (bool value : present) {
		count += value ? 1U : 0U;
	}
	return count;
}

int main() {
	using namespace kripto;
	using namespace test_support;

	const std::vector<std::uint8_t> plain(1 << 16, 0x41);
	for (CipherMode mode : allModes()) {
		const auto ciphertext = encryptBuffer(plain, key, mode, iv);
		if (decryptBuffer(ciphertext, key, mode, iv) != plain) {
			std::cerr << modeName(mode) << " histogram round-trip gagal\n";
			return 1;
		}
		std::cout << modeName(mode) << ": plaintext "
			<< distinctByteCount(plain) << " byte unik, ciphertext "
			<< distinctByteCount(ciphertext) << " byte unik\n";
	}
	std::cout << "Histogram test berhasil\n";
}
