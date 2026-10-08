#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$DIR/../.."

echo "Compiling cipher.dll..."
g++ -std=c++20 -O2 -shared -fPIC -I"$ROOT/include" \
    "$DIR/cipher_api.cpp" \
    "$ROOT/src/block_cipher.cpp" \
    "$ROOT/src/operations.cpp" \
    "$ROOT/src/padding.cpp" \
    "$ROOT/src/sbox.cpp" \
    -o "$DIR/cipher.dll"

echo "cipher.dll siap"
