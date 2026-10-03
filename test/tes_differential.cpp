#include <iostream>
#include <iomanip>
#include "../src/template_cipher.cpp"

void printBlockHex(const char* label, const kripto::Block128& b) {
	std::cout << std::left << std::setw(20) << label << ": 0x" 
	          << std::hex << std::setfill('0') 
	          << std::setw(16) << b.hi << std::setw(16) << b.lo 
	          << std::dec << std::setfill(' ') << "\n";
}

int main() {
	using namespace kripto;
	const Block128 key{0xA5A5A5A5A5A5A5A5ULL, 0x5A5A5A5A5A5A5A5AULL};
	const Block128 plain{0x0123456789ABCDEFULL, 0x0F1E2D3C4B5A6978ULL};
	const Block128 delta{0x0000000000000001ULL, 0x0000000000000000ULL};
	
	const Block128 plainWithDelta{plain.hi ^ delta.hi, plain.lo ^ delta.lo};
	const Block128 result = encryptBlock(plain, key);
	const Block128 resultWithDelta = encryptBlock(plainWithDelta, key);
	
	const Block128 cipherDelta{result.hi ^ resultWithDelta.hi, result.lo ^ resultWithDelta.lo};
	const unsigned diffBits = hammingDistance(result, resultWithDelta);

	std::cout << "==================================================\n";
	std::cout << "         ANALISIS DIFFERENTIAL CRYPTANALYSIS      \n";
	std::cout << "==================================================\n";
	printBlockHex("Key", key);
	printBlockHex("Plaintext (P1)", plain);
	printBlockHex("Input Delta (dP)", delta);
	printBlockHex("Plaintext Delta (P2)", plainWithDelta);
	std::cout << "--------------------------------------------------\n";
	printBlockHex("Ciphertext 1 (C1)", result);
	printBlockHex("Ciphertext 2 (C2)", resultWithDelta);
	printBlockHex("Output Delta (dC)", cipherDelta);
	std::cout << "--------------------------------------------------\n";
	std::cout << "Hamming Distance : " << diffBits << " bit berbeda\n";

	if (!(decryptBlock(result, key) == plain) || diffBits == 0) {
		std::cerr << "\n[STATUS] Differential test: GAGAL\n";
		return 1;
	}
	std::cout << "[STATUS] Differential test: BERHASIL (Perubahan terdeteksi)\n";
	std::cout << "==================================================\n";
}