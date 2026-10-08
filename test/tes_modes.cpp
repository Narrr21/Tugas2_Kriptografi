#include <iostream>
#include <vector>
#include "test_helpers.hpp"

int main() {
	using namespace kripto;
	using namespace test_support;

	const std::vector<std::size_t> sizes{0, 1, 15, 16, 17, 31, 32, 100, 1000};
	for (CipherMode mode : allModes()) {
		for (std::size_t size : sizes) {
			const auto plain = makeData(size, 0x4000 + size);
			const auto ciphertext = encryptBuffer(plain, key, mode, iv);
			const auto recovered = decryptBuffer(ciphertext, key, mode, iv);
			const bool padded = mode == CipherMode::ECB || mode == CipherMode::CBC;
			const bool validLength = padded
				? ciphertext.size() >= 16 && ciphertext.size() % 16 == 0
				: ciphertext.size() == plain.size();
			if (recovered != plain || !validLength) {
				std::cerr << modeName(mode) << " gagal pada ukuran " << size << "\n";
				return 1;
			}
		}
		std::cout << modeName(mode) << ": round-trip dan panjang ciphertext valid\n";
	}
	std::cout << "Mode operation test berhasil\n";
}
