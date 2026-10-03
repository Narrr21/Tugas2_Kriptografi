$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (!(Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

Write-Host "Compiling programs..."
g++ -std=c++17 test/tes_avalanche.cpp -o bin/tes_avalanche
g++ -std=c++17 test/tes_differential.cpp -o bin/tes_differential
g++ -std=c++17 test/tes_linear.cpp -o bin/tes_linear

Write-Host ""
Write-Host "Running Avalanche test..."
.\bin\tes_avalanche

Write-Host ""
Write-Host "Running Differential test..."
.\bin\tes_differential

Write-Host ""
Write-Host "Running Linear test..."
.\bin\tes_linear
