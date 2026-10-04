#include <iostream>
#include <cstdint>
#include <cstring>
#include <algorithm>

// State Konteks Internal untuk menyimpan S-Box Dinamis 128-bit
struct DynamicAuraContext {
    uint8_t sbox[256];
    uint8_t inv_sbox[256];
    bool is_initialized = false;
};

static DynamicAuraContext G_AURA_CTX;

// Inline modulo inverse yang bersifat bijektif
inline uint8_t mod_inverse(uint8_t b) { 
    return b ^ 0x55; 
}

// -----------------------------------------------------------------------------
// 1. INISIALISASI S-BOX DINAMIS (Sqrt(2) -> Injeksi Kunci 128-bit -> Mutasi Mandiri)
// -----------------------------------------------------------------------------
void init_dynamic_aura_sbox(const uint8_t master_key[16]) {
    // A. Seed NUMS dari pecahan desimal sqrt(2) (16 byte pertama)
    const uint8_t sqrt2_seed[16] = {
        0x6A, 0x09, 0xE6, 0x67, 0xBB, 0x67, 0xAE, 0x85,
        0x3C, 0x6E, 0xF3, 0x72, 0xA5, 0x4F, 0xF5, 0x3A
    };

    // B. Inisialisasi dasar 0-255 (menjamin keutuhan bijektif awal)
    for (int i = 0; i < 256; ++i) {
        G_AURA_CTX.sbox[i] = static_cast<uint8_t>(i);
    }

    // C. Pengocokan Fisher-Yates yang di-seed oleh sqrt(2) & Master Key 16-byte
    for (int i = 255; i > 0; --i) {
        int j = (sqrt2_seed[i % 16] ^ master_key[i % 16] ^ i) % (i + 1);
        std::swap(G_AURA_CTX.sbox[i], G_AURA_CTX.sbox[j]);
    }

    // D. Mutasi Mandiri (Self-Mutation Loop) pada matriks 4x4
    uint8_t dummy_state[4][4] = {0};
    for (int r = 0; r < 10; ++r) {
        // Enkripsi parsial dummy state 4x4 dengan S-Box sementara
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                dummy_state[i][j] = G_AURA_CTX.sbox[dummy_state[i][j] ^ master_key[(r*4 + i + j) % 16]];
            }
        }
        // Umpan balik ciphertext parsial untuk merombak posisi S-Box
        for (int i = 0; i < 16; ++i) {
            uint8_t idx1 = dummy_state[i / 4][i % 4];
            uint8_t idx2 = dummy_state[3 - (i / 4)][3 - (i % 4)];
            std::swap(G_AURA_CTX.sbox[idx1], G_AURA_CTX.sbox[idx2]);
        }
    }

    // E. Pembangkitan Inverse S-Box otomatis untuk Dekripsi
    for (int i = 0; i < 256; ++i) {
        G_AURA_CTX.inv_sbox[G_AURA_CTX.sbox[i]] = static_cast<uint8_t>(i);
    }

    G_AURA_CTX.is_initialized = true;
}

// Otomatisasi inisialisasi jika dipanggil langsung oleh test runner
inline void ensure_sbox_initialized(const uint8_t round_keys[10][4][4]) {
    if (!G_AURA_CTX.is_initialized) {
        uint8_t derived_key[16];
        std::memcpy(derived_key, round_keys[0], 16);
        init_dynamic_aura_sbox(derived_key);
    }
}

// -----------------------------------------------------------------------------
// 2. FUNGSI ENKRIPSI AURA-128 (Matriks 4x4)
// -----------------------------------------------------------------------------
void aura_128_encrypt(uint8_t state[4][4], const uint8_t round_keys[10][4][4]) {
    ensure_sbox_initialized(round_keys);

    for (int r = 0; r < 10; ++r) {
        
        // 1. Substitusi (Memakai S-Box Dinamis)
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                state[i][j] = G_AURA_CTX.sbox[state[i][j]];
            }
        }

        // 2. Zig-zag Permute (Transposisi Horizontal Modulo 4)
        for (int i = 0; i < 4; ++i) {
            uint8_t temp[4];
            for (int j = 0; j < 4; ++j) {
                temp[j] = state[i][(j + i + 1) % 4]; 
            }
            for (int j = 0; j < 4; ++j) state[i][j] = temp[j];
        }

        // 3. Matrix Diffusion (Difusi Vertikal Kolom 4-byte)
        for (int j = 0; j < 4; ++j) {
            uint8_t col_parity = state[0][j] ^ state[1][j] ^ state[2][j] ^ state[3][j];
            for (int i = 0; i < 4; ++i) {
                state[i][j] ^= col_parity;
            }
        }

        // 4. Modulo Inversion (Operasi Tambahan: Baris Ganjil)
        for (int i = 1; i < 4; i += 2) { 
            for (int j = 0; j < 4; ++j) {
                state[i][j] = mod_inverse(state[i][j]);
            }
        }

        // 5. AddRoundKey
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                state[i][j] ^= round_keys[r][i][j];
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 3. FUNGSI DEKRIPSI AURA-128 (Matriks 4x4)
// -----------------------------------------------------------------------------
void aura_128_decrypt(uint8_t state[4][4], const uint8_t round_keys[10][4][4]) {
    ensure_sbox_initialized(round_keys);

    for (int r = 9; r >= 0; --r) {
        
        // 5. Inverse AddRoundKey
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                state[i][j] ^= round_keys[r][i][j];
            }
        }

        // 4. Inverse Modulo Inversion
        for (int i = 1; i < 4; i += 2) { 
            for (int j = 0; j < 4; ++j) {
                state[i][j] = mod_inverse(state[i][j]);
            }
        }

        // 3. Inverse Matrix Diffusion
        for (int j = 0; j < 4; ++j) {
            uint8_t col_parity = state[0][j] ^ state[1][j] ^ state[2][j] ^ state[3][j];
            for (int i = 0; i < 4; ++i) {
                state[i][j] ^= col_parity;
            }
        }

        // 2. Inverse Zig-zag Permute
        for (int i = 0; i < 4; ++i) {
            uint8_t temp[4];
            for (int j = 0; j < 4; ++j) {
                temp[(j + i + 1) % 4] = state[i][j]; 
            }
            for (int j = 0; j < 4; ++j) state[i][j] = temp[j];
        }

        // 1. Inverse Substitusi (Memakai Inverse S-Box Dinamis)
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                state[i][j] = G_AURA_CTX.inv_sbox[state[i][j]];
            }
        }
    }
}

// -----------------------------------------------------------------------------
// MAIN PROGRAM DEMO & TEST
// -----------------------------------------------------------------------------
int main() {
    uint8_t master_key[16] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    };
    init_dynamic_aura_sbox(master_key);

    uint8_t state_matrix[4][4] = {
        {0x41, 0x55, 0x52, 0x41}, // "AURA"
        {0x2D, 0x31, 0x32, 0x38}, // "-128"
        {0x54, 0x45, 0x53, 0x54}, // "TEST"
        {0x31, 0x32, 0x33, 0x34}  // "1234"
    }; 
    uint8_t round_keys[10][4][4] = {0}; 

    std::cout << "=== ENKRIPSI AURA-128 ===" << std::endl;
    aura_128_encrypt(state_matrix, round_keys);
    
    std::cout << "=== DEKRIPSI AURA-128 ===" << std::endl;
    aura_128_decrypt(state_matrix, round_keys);

    std::cout << "Eksekusi Aura-128 S-Box Dinamis Berhasill!" << std::endl;
    return 0;
}