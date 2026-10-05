#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace kripto {

// Constants
constexpr int ROUNDS = 16;
constexpr std::uint64_t GOLDEN_RATIO_64 = 0x9E3779B97F4A7C15ULL;
constexpr std::uint64_t PI_CONST_64     = 0x243F6A8885A308D3ULL;

// Types & Data Structures
struct Block128 {
	std::uint64_t hi{0};
	std::uint64_t lo{0};

	bool operator==(const Block128& other) const = default;
};

enum class CipherMode {
	ECB,
	CBC,
	CFB,
	OFB,
	CTR
};

struct RoundKeys {
	std::array<std::uint64_t, ROUNDS> subkeys{};
	std::array<std::uint8_t, 256> sbox{};
};

// Conversions
Block128 bytesToBlock(const std::uint8_t* src);
void blockToBytes(Block128 b, std::uint8_t* dst);

// Core Cipher Primitives
RoundKeys keyExpansion(Block128 masterKey);
RoundKeys keyExpansion(const std::array<std::uint8_t, 16>& masterKey);

std::uint64_t roundFunction(std::uint64_t right, std::uint64_t roundKey, const std::array<std::uint8_t, 256>& sbox);

Block128 encryptBlock(Block128 plaintext, const RoundKeys& keys);
Block128 decryptBlock(Block128 ciphertext, const RoundKeys& keys);
Block128 encryptBlock(Block128 plaintext, Block128 masterKey);
Block128 decryptBlock(Block128 ciphertext, Block128 masterKey);

// Analysis Utilities
unsigned hammingDistance(Block128 left, Block128 right);

// PKCS#7 Padding
std::vector<std::uint8_t> padPKCS7(const std::vector<std::uint8_t>& data);
std::vector<std::uint8_t> unpadPKCS7(const std::vector<std::uint8_t>& data);

// Block Cipher Modes of Operation
void incrementCounter(Block128& counter);

std::vector<std::uint8_t> encryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
);

std::vector<std::uint8_t> decryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
);

// API Aliases
RoundKeys KeyExpansion(Block128 masterKey);
RoundKeys KeyExpansion(const std::array<std::uint8_t, 16>& masterKey);
Block128 EncryptBlock(Block128 plaintext, const RoundKeys& keys);
Block128 DecryptBlock(Block128 ciphertext, const RoundKeys& keys);
Block128 EncryptBlock(Block128 plaintext, Block128 masterKey);
Block128 DecryptBlock(Block128 ciphertext, Block128 masterKey);

std::vector<std::uint8_t> EncryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
);

std::vector<std::uint8_t> DecryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
);

} // namespace kripto
