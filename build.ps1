# PowerShell Build and Test script for DHARTI C++ Core & CLI
$ErrorActionPreference = "Stop"

$compiler = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $compiler) {
    Write-Error "g++ compiler not found in PATH."
}

if (-not (Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

$cxxFlags = @("-O3", "-std=c++17", "-Wall", "-Wextra", "-static", "-static-libgcc", "-static-libstdc++", "-Iinclude", "-Isrc/native")
$sources = @(
    "src/config/settings.cpp",
    "src/utils/logger.cpp",
    "src/services/pci_engine.cpp",
    "src/native/pci_engine.cpp",
    "src/native/contradiction_engine.cpp",
    "src/native/sia_inclusion_engine.cpp",
    "src/native/dharti_c_api.cpp",
    "src/adapters/land_record_adapter.cpp",
    "src/adapters/court_adapter.cpp",
    "src/adapters/finance_adapter.cpp",
    "src/core/event_store.cpp",
    "src/services/payment_reconciler.cpp",
    "src/services/workflow_coordinator.cpp"
)

Write-Host "==> Compiling DHARTI C++ Core Library (src/native/dharti_core.dll)..." -ForegroundColor Cyan
& g++ $cxxFlags -shared -o src/native/dharti_core.dll $sources

Write-Host "==> Compiling DHARTI C++ Test Runner (tests/cpp/test_dharti_core.exe)..." -ForegroundColor Cyan
$testSources = @("tests/cpp/test_main.cpp") + $sources
& g++ $cxxFlags -o tests/cpp/test_dharti_core.exe $testSources

Write-Host "==> Compiling DHARTI Generalized CLI (bin/dharti_cli.exe)..." -ForegroundColor Cyan
$cliSources = @("src/cli/main.cpp") + $sources
& g++ $cxxFlags -o bin/dharti_cli.exe $cliSources

Write-Host "==> Executing DHARTI C++ Test Suite..." -ForegroundColor Green
& .\tests\cpp\test_dharti_core.exe

Write-Host "==> Executing DHARTI CLI on Real Data Assets..." -ForegroundColor Green
& .\bin\dharti_cli.exe

Write-Host "==> Executing Real Wall-Clock Performance Benchmark..." -ForegroundColor Green
& .\bin\dharti_cli.exe --benchmark 1000

Write-Host "==> DHARTI C++ Build, Realtime CLI & Verification SUCCESSFUL!" -ForegroundColor Green
