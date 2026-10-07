#if __has_include("block_cipher.hpp")
#include "block_cipher.hpp"
#else
#include "../include/block_cipher.hpp"
#endif

#include <algorithm>
#include <stdexcept>

namespace kripto {

RoundKeys keyExpansion(Block128 masterKey) {
	RoundKeys rk;
	std::array<std::uint8_t, 16> kbytes{};
	blockToBytes(masterKey, kbytes.data());

	// Key-dependent S-box generation (Fisher-Yates permutation keyed with master key)
	rk.sbox = generateSBox(kbytes);

	// 16 Round keys derivation via bitwise rotations and constants
	std::uint64_t k0 = masterKey.hi;
	std::uint64_t k1 = masterKey.lo;
	for (unsigned r = 0; r < ROUNDS; ++r) {
		k0 = rotl64(k0, 19) ^ (k1 + GOLDEN_RATIO_64 + (static_cast<std::uint64_t>(r) * 0x517CC1B727220A95ULL));
		k1 = rotr64(k1, 23) + (k0 ^ PI_CONST_64);
		rk.subkeys[r] = k0 ^ rotl64(k1, 13);
	}
	return rk;
}

RoundKeys keyExpansion(const std::array<std::uint8_t, 16>& masterKey) {
	return keyExpansion(bytesToBlock(masterKey.data()));
}

std::uint64_t roundFunction(std::uint64_t right, std::uint64_t roundKey, const std::array<std::uint8_t, 256>& sbox) {
	// Modulo 2^64 addition with round key
	std::uint64_t x = addMod64(right, roundKey);

	// Bit transposition using bitwise rotations
	x = bitTransposition(x);

	// Key-dependent S-box byte substitution
	std::uint64_t sub = substituteBytes(x, sbox);

	// Non-linear mixing & diffusion
	return diffuse(sub, roundKey);
}

Block128 encryptBlock(Block128 plaintext, const RoundKeys& keys) {
	std::uint64_t left = plaintext.hi;
	std::uint64_t right = plaintext.lo;

	for (int r = 0; r < ROUNDS; ++r) {
		std::uint64_t temp = left ^ roundFunction(right, keys.subkeys[r], keys.sbox);
		left = right;
		right = temp;
	}
	return {right, left};
}

Block128 decryptBlock(Block128 ciphertext, const RoundKeys& keys) {
	std::uint64_t left = ciphertext.hi;
	std::uint64_t right = ciphertext.lo;

	for (int r = ROUNDS - 1; r >= 0; --r) {
		std::uint64_t temp = left ^ roundFunction(right, keys.subkeys[r], keys.sbox);
		left = right;
		right = temp;
	}
	return {right, left};
}

Block128 encryptBlock(Block128 plaintext, Block128 masterKey) {
	return encryptBlock(plaintext, keyExpansion(masterKey));
}

Block128 decryptBlock(Block128 ciphertext, Block128 masterKey) {
	return decryptBlock(ciphertext, keyExpansion(masterKey));
}

void incrementCounter(Block128& counter) {
	if (++counter.lo == 0) {
		++counter.hi;
	}
}

std::vector<std::uint8_t> encryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode,
	Block128 iv_or_counter
) {
	RoundKeys keys = keyExpansion(masterKey);
	std::vector<std::uint8_t> result;

	switch (mode) {
		case CipherMode::ECB: {
			std::vector<std::uint8_t> padded = padPKCS7(data);
			result.resize(padded.size());
			for (std::size_t i = 0; i < padded.size(); i += 16) {
				Block128 p = bytesToBlock(&padded[i]);
				Block128 c = encryptBlock(p, keys);
				blockToBytes(c, &result[i]);
			}
			break;
		}
		case CipherMode::CBC: {
			std::vector<std::uint8_t> padded = padPKCS7(data);
			result.resize(padded.size());
			Block128 prev = iv_or_counter;
			for (std::size_t i = 0; i < padded.size(); i += 16) {
				Block128 p = bytesToBlock(&padded[i]);
				Block128 toEnc{p.hi ^ prev.hi, p.lo ^ prev.lo};
				Block128 c = encryptBlock(toEnc, keys);
				blockToBytes(c, &result[i]);
				prev = c;
			}
			break;
		}
		case CipherMode::CFB: {
			result.resize(data.size());
			Block128 feedback = iv_or_counter;
			std::array<std::uint8_t, 16> stream{};
			for (std::size_t i = 0; i < data.size(); i += 16) {
				Block128 enc = encryptBlock(feedback, keys);
				blockToBytes(enc, stream.data());
				std::size_t chunk = std::min(std::size_t{16}, data.size() - i);
				std::array<std::uint8_t, 16> cipherChunk{};
				for (std::size_t j = 0; j < chunk; ++j) {
					result[i + j] = data[i + j] ^ stream[j];
					cipherChunk[j] = result[i + j];
				}
				for (std::size_t j = chunk; j < 16; ++j) {
					cipherChunk[j] = 0;
				}
				feedback = bytesToBlock(cipherChunk.data());
			}
			break;
		}
		case CipherMode::OFB: {
			result.resize(data.size());
			Block128 feedback = iv_or_counter;
			std::array<std::uint8_t, 16> stream{};
			for (std::size_t i = 0; i < data.size(); i += 16) {
				feedback = encryptBlock(feedback, keys);
				blockToBytes(feedback, stream.data());
				std::size_t chunk = std::min(std::size_t{16}, data.size() - i);
				for (std::size_t j = 0; j < chunk; ++j) {
					result[i + j] = data[i + j] ^ stream[j];
				}
			}
			break;
		}
		case CipherMode::CTR: {
			result.resize(data.size());
			Block128 ctr = iv_or_counter;
			std::array<std::uint8_t, 16> stream{};
			for (std::size_t i = 0; i < data.size(); i += 16) {
				Block128 enc = encryptBlock(ctr, keys);
				blockToBytes(enc, stream.data());
				std::size_t chunk = std::min(std::size_t{16}, data.size() - i);
				for (std::size_t j = 0; j < chunk; ++j) {
					result[i + j] = data[i + j] ^ stream[j];
				}
				incrementCounter(ctr);
			}
			break;
		}
	}
	return result;
}

std::vector<std::uint8_t> decryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode,
	Block128 iv_or_counter
) {
	RoundKeys keys = keyExpansion(masterKey);
	std::vector<std::uint8_t> result;

	switch (mode) {
		case CipherMode::ECB: {
			if (data.empty() || (data.size() % 16) != 0) {
				throw std::runtime_error("Invalid ECB ciphertext length");
			}
			std::vector<std::uint8_t> decrypted(data.size());
			for (std::size_t i = 0; i < data.size(); i += 16) {
				Block128 c = bytesToBlock(&data[i]);
				Block128 p = decryptBlock(c, keys);
				blockToBytes(p, &decrypted[i]);
			}
			result = unpadPKCS7(decrypted);
			break;
		}
		case CipherMode::CBC: {
			if (data.empty() || (data.size() % 16) != 0) {
				throw std::runtime_error("Invalid CBC ciphertext length");
			}
			std::vector<std::uint8_t> decrypted(data.size());
			Block128 prev = iv_or_counter;
			for (std::size_t i = 0; i < data.size(); i += 16) {
				Block128 c = bytesToBlock(&data[i]);
				Block128 p = decryptBlock(c, keys);
				Block128 plain{p.hi ^ prev.hi, p.lo ^ prev.lo};
				blockToBytes(plain, &decrypted[i]);
				prev = c;
			}
			result = unpadPKCS7(decrypted);
			break;
		}
		case CipherMode::CFB: {
			result.resize(data.size());
			Block128 feedback = iv_or_counter;
			std::array<std::uint8_t, 16> stream{};
			for (std::size_t i = 0; i < data.size(); i += 16) {
				Block128 enc = encryptBlock(feedback, keys);
				blockToBytes(enc, stream.data());
				std::size_t chunk = std::min(std::size_t{16}, data.size() - i);
				std::array<std::uint8_t, 16> cipherChunk{};
				for (std::size_t j = 0; j < chunk; ++j) {
					result[i + j] = data[i + j] ^ stream[j];
					cipherChunk[j] = data[i + j];
				}
				for (std::size_t j = chunk; j < 16; ++j) {
					cipherChunk[j] = 0;
				}
				feedback = bytesToBlock(cipherChunk.data());
			}
			break;
		}
		case CipherMode::OFB: {
			return encryptBuffer(data, masterKey, CipherMode::OFB, iv_or_counter);
		}
		case CipherMode::CTR: {
			return encryptBuffer(data, masterKey, CipherMode::CTR, iv_or_counter);
		}
	}
	return result;
}

// ============================================================================
// API Aliases
// ============================================================================

RoundKeys KeyExpansion(Block128 masterKey) {
	return keyExpansion(masterKey);
}

RoundKeys KeyExpansion(const std::array<std::uint8_t, 16>& masterKey) {
	return keyExpansion(masterKey);
}

Block128 EncryptBlock(Block128 plaintext, const RoundKeys& keys) {
	return encryptBlock(plaintext, keys);
}

Block128 DecryptBlock(Block128 ciphertext, const RoundKeys& keys) {
	return decryptBlock(ciphertext, keys);
}

Block128 EncryptBlock(Block128 plaintext, Block128 masterKey) {
	return encryptBlock(plaintext, masterKey);
}

Block128 DecryptBlock(Block128 ciphertext, Block128 masterKey) {
	return decryptBlock(ciphertext, masterKey);
}

std::vector<std::uint8_t> EncryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode,
	Block128 iv_or_counter
) {
	return encryptBuffer(data, masterKey, mode, iv_or_counter);
}

std::vector<std::uint8_t> DecryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode,
	Block128 iv_or_counter
) {
	return decryptBuffer(data, masterKey, mode, iv_or_counter);
}

} // namespace kripto
