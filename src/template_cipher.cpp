#include <array>
#include <cstdint>

namespace kripto {

struct Block128 {
	std::uint64_t hi;
	std::uint64_t lo;

	bool operator==(const Block128& other) const {
		return hi == other.hi && lo == other.lo;
	}
};

constexpr int ROUNDS = 16;

// A small 4-bit lookup table keeps the substitution easy to inspect and invert.
constexpr std::array<std::uint8_t, 16> SBOX = {
	0xE, 0x4, 0xD, 0x1, 0x2, 0xF, 0xB, 0x8,
	0x3, 0xA, 0x6, 0xC, 0x5, 0x9, 0x0, 0x7
};

constexpr std::array<std::uint8_t, 16> INV_SBOX = {
	0xE, 0x3, 0x4, 0x8, 0x1, 0xC, 0xA, 0xF,
	0x7, 0xD, 0x9, 0x6, 0xB, 0x2, 0x0, 0x5
};

inline std::uint64_t rotl64(std::uint64_t value, unsigned amount) {
	amount %= 64;
	return amount == 0 ? value : (value << amount) | (value >> (64 - amount));
}

inline Block128 rotl128(Block128 value, unsigned amount) {
	amount %= 128;
	if (amount == 0) {
		return value;
	}
	if (amount < 64) {
		return {
			(value.hi << amount) | (value.lo >> (64 - amount)),
			(value.lo << amount) | (value.hi >> (64 - amount))
		};
	}
	amount -= 64;
	return {
		(value.lo << amount) | (value.hi >> (64 - amount)),
		(value.hi << amount) | (value.lo >> (64 - amount))
	};
}

inline bool getBit(Block128 value, unsigned position) {
	return position < 64
		? ((value.hi >> (63 - position)) & 1U) != 0
		: ((value.lo >> (127 - position)) & 1U) != 0;
}

inline void setBit(Block128& value, unsigned position) {
	if (position < 64) {
		value.hi |= std::uint64_t{1} << (63 - position);
	} else {
		value.lo |= std::uint64_t{1} << (127 - position);
	}
}

inline Block128 substitute(Block128 value, const std::array<std::uint8_t, 16>& table) {
	Block128 result{};
	for (unsigned position = 0; position < 32; ++position) {
		std::uint8_t nibble = position < 16
			? static_cast<std::uint8_t>((value.hi >> (60 - position * 4)) & 0xF)
			: static_cast<std::uint8_t>((value.lo >> (60 - (position - 16) * 4)) & 0xF);
		std::uint64_t replacement = table[nibble];
		if (position < 16) {
			result.hi |= replacement << (60 - position * 4);
		} else {
			result.lo |= replacement << (60 - (position - 16) * 4);
		}
	}
	return result;
}

// Bit transposition: position i moves to 5*i modulo 127; bit 127 stays fixed.
inline Block128 permuteBits(Block128 value) {
	Block128 result{};
	for (unsigned position = 0; position < 127; ++position) {
		if (getBit(value, position)) {
			setBit(result, (position * 5) % 127);
		}
	}
	if (getBit(value, 127)) {
		setBit(result, 127);
	}
	return result;
}

inline Block128 inversePermuteBits(Block128 value) {
	Block128 result{};
	for (unsigned position = 0; position < 127; ++position) {
		if (getBit(value, position)) {
			setBit(result, (position * 51) % 127);
		}
	}
	if (getBit(value, 127)) {
		setBit(result, 127);
	}
	return result;
}

inline Block128 deriveRoundKey(Block128 masterKey, unsigned round) {
	Block128 roundKey = masterKey;
	for (unsigned step = 0; step <= round; ++step) {
		roundKey = rotl128(roundKey, 13);
		roundKey.hi ^= 0x9E3779B97F4A7C15ULL + step * 0x0101010101010101ULL;
		roundKey.lo ^= 0xD1B54A32D192ED03ULL ^ (static_cast<std::uint64_t>(step) << 32);
	}
	return roundKey;
}

inline void mix(Block128& state) {
	state.lo += rotl64(state.hi, 7);
	state.hi ^= rotl64(state.lo, 11);
}

inline void inverseMix(Block128& state) {
	state.hi ^= rotl64(state.lo, 11);
	state.lo -= rotl64(state.hi, 7);
}

inline Block128 encryptBlock(Block128 plaintext, Block128 masterKey) {
	Block128 state = plaintext;
	for (unsigned round = 0; round < ROUNDS; ++round) {
		state.hi ^= deriveRoundKey(masterKey, round).hi;
		state.lo ^= deriveRoundKey(masterKey, round).lo;
		state = substitute(state, SBOX);
		state = inversePermuteBits(state);
		mix(state);
	}
	return state;
}

inline Block128 decryptBlock(Block128 ciphertext, Block128 masterKey) {
	Block128 state = ciphertext;
	for (int round = ROUNDS - 1; round >= 0; --round) {
		inverseMix(state);
		state = permuteBits(state);
		state = substitute(state, INV_SBOX);
		Block128 roundKey = deriveRoundKey(masterKey, static_cast<unsigned>(round));
		state.hi ^= roundKey.hi;
		state.lo ^= roundKey.lo;
	}
	return state;
}

inline unsigned hammingDistance(Block128 left, Block128 right) {
	return static_cast<unsigned>(__builtin_popcountll(left.hi ^ right.hi)
		+ __builtin_popcountll(left.lo ^ right.lo));
}

} // namespace kripto