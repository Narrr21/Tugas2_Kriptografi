$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$dll = Join-Path $root "cipher.dll"
$sources = @(
    (Join-Path $root "cipher_api.cpp"),
    (Join-Path $root "..\block_cipher.cpp"),
    (Join-Path $root "..\operations.cpp"),
    (Join-Path $root "..\padding.cpp"),
    (Join-Path $root "..\sbox.cpp")
)

$stale = !(Test-Path $dll) -or ($sources | Where-Object { (Get-Item $_).LastWriteTime -gt (Get-Item $dll).LastWriteTime })
if ($stale) {
    & (Join-Path $root "build.ps1")
}

python -m streamlit run (Join-Path $root "app.py")
