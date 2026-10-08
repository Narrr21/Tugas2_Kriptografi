#pragma once

#if __has_include("sbox.hpp")
#include "sbox.hpp"
#include "operations.hpp"
#include "padding.hpp"
#else
#include "../include/sbox.hpp"
#include "../include/operations.hpp"
#include "../include/padding.hpp"
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace kripto {

// Constants
constexpr int ROUNDS = 16;
constexpr std::uint64_t GOLDEN_RATIO_64 = 0x9E3779B97F4A7C15ULL;
constexpr std::uint64_t PI_CONST_64     = 0x243F6A8885A308D3ULL;

// Cipher Modes
enum class CipherMode {
	ECB,
	CBC,
	CFB,
	OFB,
	CTR
};

// Round Keys Structure
struct RoundKeys {
	std::array<std::uint64_t, ROUNDS> subkeys{};
	std::array<std::uint8_t, 256> sbox{};
};

// Core Cipher Primitives
RoundKeys keyExpansion(Block128 masterKey);
RoundKeys keyExpansion(const std::array<std::uint8_t, 16>& masterKey);

std::uint64_t roundFunction(std::uint64_t right, std::uint64_t roundKey, const std::array<std::uint8_t, 256>& sbox);

Block128 encryptBlock(Block128 plaintext, const RoundKeys& keys);
Block128 decryptBlock(Block128 ciphertext, const RoundKeys& keys);
Block128 encryptBlock(Block128 plaintext, Block128 masterKey);
Block128 decryptBlock(Block128 ciphertext, Block128 masterKey);

// Modes of Operation
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
