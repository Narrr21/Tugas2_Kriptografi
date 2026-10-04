$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (!(Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

Write-Host "Compiling programs..."
g++ -std=c++17 $CipherDefine test/tes_avalanche.cpp -o bin/tes_avalanche
g++ -std=c++17 $CipherDefine test/tes_differential.cpp -o bin/tes_differential
g++ -std=c++17 $CipherDefine test/tes_entropi.cpp -o bin/tes_entropi
g++ -std=c++17 $CipherDefine test/tes_histogram.cpp -o bin/tes_histogram

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
