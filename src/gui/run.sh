#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$DIR/../.."
LIB="$DIR/cipher.dll"

SRCS=(
    "$DIR/cipher_api.cpp"
    "$ROOT/src/block_cipher.cpp"
    "$ROOT/src/operations.cpp"
    "$ROOT/src/padding.cpp"
    "$ROOT/src/sbox.cpp"
)

STALE=0
if [ ! -f "$LIB" ]; then
    STALE=1
else
    for src in "${SRCS[@]}"; do
        if [ "$src" -nt "$LIB" ]; then
            STALE=1
            break
        fi
    done
fi

if [ "$STALE" -eq 1 ]; then
    "$DIR/build.sh"
fi

PYTHON="${PYTHON:-python3}"
"$PYTHON" -m streamlit run "$DIR/app.py" "$@"
