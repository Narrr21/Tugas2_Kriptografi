#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"
mkdir -p bin

SRCS="src/sbox.cpp src/operations.cpp src/padding.cpp src/block_cipher.cpp"

echo "Compiling programs..."
for test in avalanche differential entropi histogram; do
    g++ -std=c++20 ${CIPHER_DEFINE:-} -Iinclude "test/tes_${test}.cpp" $SRCS -o "bin/tes_${test}"
done

for test in avalanche differential entropi histogram; do
    echo ""
    echo "Running ${test^} test..."
    "./bin/tes_${test}"
done
