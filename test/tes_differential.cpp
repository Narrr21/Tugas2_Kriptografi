#include <iomanip>
#include <iostream>
#include <vector>
#include "test_helpers.hpp"

int main() {
	using namespace kripto;
	using namespace test_support;

	unsigned totalChanged = 0;
	unsigned totalBits = 0;
	const auto delta = std::uint8_t{0x01};

	for (CipherMode mode : allModes()) {
		unsigned modeChanged = 0;
		unsigned modeBits = 0;
		for (unsigned trial = 0; trial < 100; ++trial) {
			auto plain = makeData(64, 0x2000 + trial);
			auto changed = plain;
			changed[(trial * 13) % changed.size()] ^= delta;
			const auto first = encryptBuffer(plain, key, mode, iv);
			const auto second = encryptBuffer(changed, key, mode, iv);
			if (decryptBuffer(first, key, mode, iv) != plain
				|| decryptBuffer(second, key, mode, iv) != changed) {
				std::cerr << modeName(mode) << " round-trip gagal\n";
				return 1;
			}
			const unsigned bits = static_cast<unsigned>(first.size() * 8);
			const unsigned changedBits = hammingDistance(first, second);
			modeChanged += changedBits;
			modeBits += bits;
			totalChanged += changedBits;
			totalBits += bits;
		}
		std::cout << modeName(mode) << ": " << std::fixed << std::setprecision(2)
			<< (100.0 * modeChanged / modeBits) << "% ("
			<< modeChanged << "/" << modeBits << " bit)\n";
	}

	std::cout << "Differential rata-rata: " << std::fixed << std::setprecision(2)
		<< (100.0 * totalChanged / totalBits) << "%\n";
	if (totalChanged == 0) {
		std::cerr << "Differential test gagal\n";
		return 1;
	}
	std::cout << "Differential test berhasil\n";
}
