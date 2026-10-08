$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path

$src = Join-Path $root ".."
$inc = Join-Path $root "..\..\include"

Write-Host "Compiling cipher.dll..."
g++ -std=c++20 -O2 -shared -static -I"$inc" -o "$root\cipher.dll" `
    "$root\cipher_api.cpp" `
    "$src\block_cipher.cpp" `
    "$src\operations.cpp" `
    "$src\padding.cpp" `
    "$src\sbox.cpp"
if ($LASTEXITCODE -ne 0) {
    throw "Kompilasi cipher.dll gagal"
}
Write-Host "cipher.dll siap"
