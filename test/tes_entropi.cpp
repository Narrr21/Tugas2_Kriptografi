#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include "../include/block_cipher.hpp"


double byteEntropy(const kripto::Block128& block) {
	std::uint8_t bytes[16];
	for (unsigned index = 0; index < 8; ++index) {
		bytes[index] = static_cast<std::uint8_t>(block.hi >> (56 - index * 8));
		bytes[index + 8] = static_cast<std::uint8_t>(block.lo >> (56 - index * 8));
	}

	unsigned frequencies[256] = {};
	for (std::uint8_t byte : bytes) {
		++frequencies[byte];
	}

	double entropy = 0.0;
	for (unsigned frequency : frequencies) {
		if (frequency != 0) {
			const double probability = frequency / 16.0;
			entropy -= probability * std::log2(probability);
		}
	}
	return entropy;
}

int main() {
	using namespace kripto;
	const Block128 key{0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL};
	const Block128 plain{0x1111111111111111ULL, 0x2222222222222222ULL};
	const Block128 ciphertext = encryptBlock(plain, key);
	const double entropy = byteEntropy(ciphertext);

	std::cout << "==================================================\n";
	std::cout << "                ANALISIS ENTROPI BYTE             \n";
	std::cout << "==================================================\n";
	std::cout << "Entropy ciphertext: " << std::fixed << std::setprecision(4)
	          << entropy << " bit/byte\n";
	std::cout << "Maksimum teoritis : 8.0000 bit/byte\n";

	if (!(decryptBlock(ciphertext, key) == plain) || entropy <= 0.0 || entropy > 8.0) {
		std::cerr << "[STATUS] Entropy test: GAGAL\n";
		return 1;
	}
	std::cout << "[STATUS] Entropy test: BERHASIL\n";
	std::cout << "==================================================\n";
}
