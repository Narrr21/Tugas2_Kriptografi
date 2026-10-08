#include <iostream>
#include "test_helpers.hpp"

int main() {
	using namespace kripto;
	using namespace test_support;

	for (unsigned trial = 0; trial < 1000; ++trial) {
		const auto plain = makeData(16, 0x3000 + trial);
		const auto ciphertext = encryptBlock(bytesToBlock(plain.data()), key);
		if (decryptBlock(ciphertext, key) != bytesToBlock(plain.data())) {
			std::cerr << "Round-trip blok gagal pada percobaan " << trial << "\n";
			return 1;
		}
	}
	std::cout << "Round-trip blok 128-bit berhasil untuk 1000 percobaan\n";
}
