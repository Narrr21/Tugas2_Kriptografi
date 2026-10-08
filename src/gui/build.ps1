$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host "Compiling cipher.dll..."
g++ -std=c++20 -O2 -shared -static -o "$root\cipher.dll" "$root\cipher_api.cpp"
if ($LASTEXITCODE -ne 0) {
    throw "Kompilasi cipher.dll gagal"
}
Write-Host "cipher.dll siap"
