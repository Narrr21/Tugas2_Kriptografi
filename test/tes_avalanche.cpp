#include <iostream>
#include <iomanip>
#include "../src/template_cipher.cpp"

void printBlockHex(const char* label, const kripto::Block128& b) {
	std::cout << std::left << std::setw(18) << label << ": 0x" 
	          << std::hex << std::setfill('0') 
	          << std::setw(16) << b.hi << std::setw(16) << b.lo 
	          << std::dec << std::setfill(' ') << "\n";
}

int main() {
	using namespace kripto;
	const Block128 key{0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL};
	const Block128 plain{0x1111111111111111ULL, 0x2222222222222222ULL};
	const Block128 changed{plain.hi ^ (1ULL << 17), plain.lo};

	const Block128 first = encryptBlock(plain, key);
	const Block128 second = encryptBlock(changed, key);
	const unsigned changedBits = hammingDistance(first, second);
	const double percentage = (changedBits / 128.0) * 100.0;

	std::cout << "==================================================\n";
	std::cout << "             ANALISIS AVALANCHE EFFECT            \n";
	std::cout << "==================================================\n";
	printBlockHex("Key", key);
	printBlockHex("Plaintext 1", plain);
	printBlockHex("Plaintext 2 (1-bit)", changed);
	std::cout << "--------------------------------------------------\n";
	printBlockHex("Ciphertext 1", first);
	printBlockHex("Ciphertext 2", second);
	std::cout << "--------------------------------------------------\n";
	std::cout << "Perubahan Bit    : " << changedBits << " / 128 bit\n";
	std::cout << "Persentase       : " << std::fixed << std::setprecision(2) << percentage << "%\n";
	std::cout << "Ideal Avalanche  : ~50.00% (64 bit)\n";

	if (!(decryptBlock(first, key) == plain) || changedBits == 0) {
		std::cerr << "\n[STATUS] Avalanche test: GAGAL / Decrypt Mismatch\n";
		return 1;
	}
	std::cout << "[STATUS] Avalanche test: BERHASIL\n";
	std::cout << "==================================================\n";
}