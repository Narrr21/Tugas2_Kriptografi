#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace kripto {

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

constexpr int ROUNDS = 16;
constexpr std::uint64_t GOLDEN_RATIO_64 = 0x9E3779B97F4A7C15ULL;
constexpr std::uint64_t PI_CONST_64     = 0x243F6A8885A308D3ULL;

struct RoundKeys {
	std::array<std::uint64_t, ROUNDS> subkeys{};
	std::array<std::uint8_t, 256> sbox{};
};

inline Block128 bytesToBlock(const std::uint8_t* src) {
	Block128 b{};
	for (int i = 0; i < 8; ++i) {
		b.hi = (b.hi << 8) | src[i];
	}
	for (int i = 8; i < 16; ++i) {
		b.lo = (b.lo << 8) | src[i];
	}
	return b;
}

inline void blockToBytes(Block128 b, std::uint8_t* dst) {
	for (int i = 7; i >= 0; --i) {
		dst[i] = static_cast<std::uint8_t>(b.hi & 0xFF);
		b.hi >>= 8;
	}
	for (int i = 15; i >= 8; --i) {
		dst[i] = static_cast<std::uint8_t>(b.lo & 0xFF);
		b.lo >>= 8;
	}
}

inline RoundKeys keyExpansion(Block128 masterKey) {
	RoundKeys rk;
	std::array<std::uint8_t, 16> kbytes{};
	blockToBytes(masterKey, kbytes.data());

	// Key-dependent S-box generation (Fisher-Yates permutation keyed with master key)
	for (int i = 0; i < 256; ++i) {
		rk.sbox[i] = static_cast<std::uint8_t>(i);
	}
	std::uint8_t j = 0;
	for (int i = 0; i < 256; ++i) {
		j = static_cast<std::uint8_t>(j + rk.sbox[i] + kbytes[i % 16] + 0x5A);
		std::swap(rk.sbox[i], rk.sbox[j]);
	}

	// 16 Round keys derivation via bitwise rotations and constants
	std::uint64_t k0 = masterKey.hi;
	std::uint64_t k1 = masterKey.lo;
	for (unsigned r = 0; r < ROUNDS; ++r) {
		k0 = std::rotl(k0, 19) ^ (k1 + GOLDEN_RATIO_64 + (static_cast<std::uint64_t>(r) * 0x517CC1B727220A95ULL));
		k1 = std::rotr(k1, 23) + (k0 ^ PI_CONST_64);
		rk.subkeys[r] = k0 ^ std::rotl(k1, 13);
	}
	return rk;
}

inline RoundKeys keyExpansion(const std::array<std::uint8_t, 16>& masterKey) {
	return keyExpansion(bytesToBlock(masterKey.data()));
}

inline std::uint64_t roundFunction(std::uint64_t right, std::uint64_t roundKey, const std::array<std::uint8_t, 256>& sbox) {
	// Modulo 2^64 addition with round key
	std::uint64_t x = right + roundKey;

	// Bit transposition using std::rotl and std::rotr
	x = std::rotl(x, 17) ^ std::rotr(x, 29);

	// Key-dependent S-box byte substitution
	std::uint64_t sub = 0;
	for (int i = 0; i < 8; ++i) {
		std::uint8_t b = static_cast<std::uint8_t>(x >> (i * 8));
		sub |= static_cast<std::uint64_t>(sbox[b]) << (i * 8);
	}

	// Non-linear mixing & diffusion
	sub = std::rotl(sub, 31);
	sub += roundKey;
	sub ^= std::rotr(sub, 13);
	return sub;
}

inline Block128 encryptBlock(Block128 plaintext, const RoundKeys& keys) {
	std::uint64_t left = plaintext.hi;
	std::uint64_t right = plaintext.lo;

	for (int r = 0; r < ROUNDS; ++r) {
		std::uint64_t temp = left ^ roundFunction(right, keys.subkeys[r], keys.sbox);
		left = right;
		right = temp;
	}
	return {right, left};
}

inline Block128 decryptBlock(Block128 ciphertext, const RoundKeys& keys) {
	std::uint64_t left = ciphertext.hi;
	std::uint64_t right = ciphertext.lo;

	for (int r = ROUNDS - 1; r >= 0; --r) {
		std::uint64_t temp = left ^ roundFunction(right, keys.subkeys[r], keys.sbox);
		left = right;
		right = temp;
	}
	return {right, left};
}

inline Block128 encryptBlock(Block128 plaintext, Block128 masterKey) {
	return encryptBlock(plaintext, keyExpansion(masterKey));
}

inline Block128 decryptBlock(Block128 ciphertext, Block128 masterKey) {
	return decryptBlock(ciphertext, keyExpansion(masterKey));
}

inline unsigned hammingDistance(Block128 left, Block128 right) {
	return static_cast<unsigned>(std::popcount(left.hi ^ right.hi) + std::popcount(left.lo ^ right.lo));
}

inline std::vector<std::uint8_t> padPKCS7(const std::vector<std::uint8_t>& data) {
	std::size_t padLen = 16 - (data.size() % 16);
	std::vector<std::uint8_t> padded = data;
	padded.insert(padded.end(), padLen, static_cast<std::uint8_t>(padLen));
	return padded;
}

inline std::vector<std::uint8_t> unpadPKCS7(const std::vector<std::uint8_t>& data) {
	if (data.empty() || (data.size() % 16) != 0) {
		throw std::runtime_error("Invalid PKCS#7 padded block alignment");
	}
	std::uint8_t padLen = data.back();
	if (padLen == 0 || padLen > 16 || padLen > data.size()) {
		throw std::runtime_error("Invalid PKCS#7 padding length");
	}
	for (std::size_t i = data.size() - padLen; i < data.size(); ++i) {
		if (data[i] != padLen) {
			throw std::runtime_error("Corrupted PKCS#7 padding bytes");
		}
	}
	return std::vector<std::uint8_t>(data.begin(), data.end() - padLen);
}

inline void incrementCounter(Block128& counter) {
	if (++counter.lo == 0) {
		++counter.hi;
	}
}

inline std::vector<std::uint8_t> encryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
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

inline std::vector<std::uint8_t> decryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
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
			// OFB decryption is identical to OFB encryption
			return encryptBuffer(data, masterKey, CipherMode::OFB, iv_or_counter);
		}
		case CipherMode::CTR: {
			// CTR decryption is identical to CTR encryption
			return encryptBuffer(data, masterKey, CipherMode::CTR, iv_or_counter);
		}
	}
	return result;
}

// API aliases matching naming conventions
inline RoundKeys KeyExpansion(Block128 masterKey) { return keyExpansion(masterKey); }
inline RoundKeys KeyExpansion(const std::array<std::uint8_t, 16>& masterKey) { return keyExpansion(masterKey); }
inline Block128 EncryptBlock(Block128 plaintext, const RoundKeys& keys) { return encryptBlock(plaintext, keys); }
inline Block128 DecryptBlock(Block128 ciphertext, const RoundKeys& keys) { return decryptBlock(ciphertext, keys); }
inline Block128 EncryptBlock(Block128 plaintext, Block128 masterKey) { return encryptBlock(plaintext, masterKey); }
inline Block128 DecryptBlock(Block128 ciphertext, Block128 masterKey) { return decryptBlock(ciphertext, masterKey); }
inline std::vector<std::uint8_t> EncryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
) {
	return encryptBuffer(data, masterKey, mode, iv_or_counter);
}
inline std::vector<std::uint8_t> DecryptBuffer(
	const std::vector<std::uint8_t>& data,
	Block128 masterKey,
	CipherMode mode = CipherMode::CBC,
	Block128 iv_or_counter = {}
) {
	return decryptBuffer(data, masterKey, mode, iv_or_counter);
}

} // namespace kripto
