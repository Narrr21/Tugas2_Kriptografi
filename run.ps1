$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (!(Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

Write-Host "Compiling programs..."
g++ -std=c++20 $CipherDefine -Iinclude test/tes_avalanche.cpp src/sbox.cpp src/operations.cpp src/padding.cpp src/block_cipher.cpp -o bin/tes_avalanche
g++ -std=c++20 $CipherDefine -Iinclude test/tes_differential.cpp src/sbox.cpp src/operations.cpp src/padding.cpp src/block_cipher.cpp -o bin/tes_differential
g++ -std=c++20 $CipherDefine -Iinclude test/tes_entropi.cpp src/sbox.cpp src/operations.cpp src/padding.cpp src/block_cipher.cpp -o bin/tes_entropi
g++ -std=c++20 $CipherDefine -Iinclude test/tes_histogram.cpp src/sbox.cpp src/operations.cpp src/padding.cpp src/block_cipher.cpp -o bin/tes_histogram

Write-Host ""
Write-Host "Running Avalanche test..."
.\bin\tes_avalanche

Write-Host ""
Write-Host "Running Differential test..."
.\bin\tes_differential

Write-Host ""
Write-Host "Running Entropy test..."
.\bin\tes_entropi

Write-Host ""
Write-Host "Running Histogram test..."
.\bin\tes_histogram
