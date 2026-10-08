#include <iomanip>
#include <iostream>
#include <vector>
#include "test_helpers.hpp"

int main() {
	using namespace kripto;
	using namespace test_support;

	const std::vector<std::size_t> sizes{16, 32, 64, 128};
	unsigned totalBits = 0;
	unsigned totalChanged = 0;

	for (CipherMode mode : allModes()) {
		unsigned modeBits = 0;
		unsigned modeChanged = 0;
		for (std::size_t size : sizes) {
			const auto plain = makeData(size, 0x1000 + size);
			auto changed = plain;
			changed[(size * 7) % size] ^= 0x01;
			const auto first = encryptBuffer(plain, key, mode, iv);
			const auto second = encryptBuffer(changed, key, mode, iv);
			if (decryptBuffer(first, key, mode, iv) != plain
				|| decryptBuffer(second, key, mode, iv) != changed) {
				std::cerr << modeName(mode) << " round-trip gagal\n";
				return 1;
			}
			const unsigned bits = static_cast<unsigned>(first.size() * 8);
			const unsigned changedBits = hammingDistance(first, second);
			modeBits += bits;
			modeChanged += changedBits;
			totalBits += bits;
			totalChanged += changedBits;
		}
		const double modePercentage = 100.0 * modeChanged / modeBits;
		std::cout << modeName(mode) << ": " << std::fixed << std::setprecision(2)
			<< modePercentage << "% (" << modeChanged << "/" << modeBits
			<< " bit)\n";
	}

	const double percentage = 100.0 * totalChanged / totalBits;
	std::cout << "Avalanche rata-rata: " << std::fixed << std::setprecision(2)
		<< percentage << "% (" << totalChanged << "/" << totalBits << " bit)\n";
	if (totalChanged == 0) {
		std::cerr << "Avalanche test gagal\n";
		return 1;
	}
	std::cout << "Avalanche test berhasil\n";
}
