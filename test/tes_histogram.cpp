#include <cstdint>
#include <iomanip>
#include <iostream>
#include "../include/block_cipher.hpp"


void printHistogram(const char* label, const kripto::Block128& block) {
	unsigned frequencies[256] = {};
	for (unsigned index = 0; index < 8; ++index) {
		++frequencies[(block.hi >> (56 - index * 8)) & 0xFFU];
		++frequencies[(block.lo >> (56 - index * 8)) & 0xFFU];
	}

	std::cout << label << " (byte:frekuensi non-zero)\n";
	for (unsigned byte = 0; byte < 256; ++byte) {
		if (frequencies[byte] != 0) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << byte
			          << std::dec << ":" << frequencies[byte] << " ";
		}
	}
	std::cout << "\n";
}

unsigned distinctByteCount(const kripto::Block128& block) {
	bool present[256] = {};
	for (unsigned index = 0; index < 8; ++index) {
		present[(block.hi >> (56 - index * 8)) & 0xFFU] = true;
		present[(block.lo >> (56 - index * 8)) & 0xFFU] = true;
	}

	unsigned count = 0;
	for (bool value : present) {
		count += value ? 1U : 0U;
	}
	return count;
}

int main() {
	using namespace kripto;
	const Block128 key{0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL};
	const Block128 plain{0x1111111111111111ULL, 0x2222222222222222ULL};
	const Block128 ciphertext = encryptBlock(plain, key);

	std::cout << "==================================================\n";
	std::cout << "              ANALISIS HISTOGRAM BYTE             \n";
	std::cout << "==================================================\n";
	printHistogram("Plaintext ", plain);
	printHistogram("Ciphertext", ciphertext);
	std::cout << "Byte berbeda plaintext : " << distinctByteCount(plain) << "\n";
	std::cout << "Byte berbeda ciphertext: " << distinctByteCount(ciphertext) << "\n";

	if (!(decryptBlock(ciphertext, key) == plain)
		|| distinctByteCount(ciphertext) <= distinctByteCount(plain)) {
		std::cerr << "[STATUS] Histogram test: GAGAL\n";
		return 1;
	}
	std::cout << "[STATUS] Histogram test: BERHASIL\n";
	std::cout << "==================================================\n";
}
