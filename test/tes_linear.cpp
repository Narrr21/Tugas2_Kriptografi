#include <iostream>
#include "../src/template_cipher.cpp"

int main() {
	using namespace kripto;
	const Block128 key{0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL};
	const Block128 plain{0xCAFEBABECAFEBABEULL, 0x1234567890ABCDEFULL};
	const Block128 ciphertext = encryptBlock(plain, key);

	// Sanity check used before collecting linear approximation statistics.
	if (!(decryptBlock(ciphertext, key) == plain)) {
		std::cerr << "Linear test gagal: round-trip tidak valid\n";
		return 1;
	}
	std::cout << "Linear: round-trip 128-bit valid\n";
}
