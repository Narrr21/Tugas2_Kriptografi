#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>
#include "test_helpers.hpp"

double byteEntropy(const std::vector<std::uint8_t>& data) {
	if (data.empty()) {
		return 0.0;
	}
	std::uint64_t frequencies[256]{};
	for (std::uint8_t value : data) {
		++frequencies[value];
	}
	double entropy = 0.0;
	for (std::uint64_t frequency : frequencies) {
		if (frequency != 0) {
			const double probability = static_cast<double>(frequency) / data.size();
			entropy -= probability * std::log2(probability);
		}
	}
	return entropy;
}

int main() {
	using namespace kripto;
	using namespace test_support;

	const auto plain = makeData(1 << 16);
	for (CipherMode mode : allModes()) {
		const auto ciphertext = encryptBuffer(plain, key, mode, iv);
		const double entropy = byteEntropy(ciphertext);
		if (decryptBuffer(ciphertext, key, mode, iv) != plain || entropy <= 0.0 || entropy > 8.0) {
			std::cerr << modeName(mode) << " entropy test gagal\n";
			return 1;
		}
		std::cout << modeName(mode) << ": " << std::fixed << std::setprecision(4)
			<< entropy << " bit/byte\n";
	}
	std::cout << "Entropy test berhasil\n";
}
