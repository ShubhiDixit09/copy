# DHARTI - Light Theme Web UI Launcher
Write-Host "================================================================================" -ForegroundColor Cyan
Write-Host "   DHARTI: National Land Acquisition Control Plane (SIH 26016)                 " -ForegroundColor Green
Write-Host "   Light Theme Interactive Web UI & Manual Testing Sandbox                     " -ForegroundColor Green
Write-Host "================================================================================" -ForegroundColor Cyan

$Port = 8080
$WebDir = Join-Path $PSScriptRoot "web"

Write-Host "[INFO] Serving UI from: $WebDir" -ForegroundColor Yellow
Write-Host "[INFO] Local URL: http://localhost:$Port" -ForegroundColor Yellow
Write-Host "[INFO] Opening default browser..." -ForegroundColor Cyan

# Launch browser
Start-Process "http://localhost:$Port"

# Run Python static server in the web directory
Set-Location -Path $WebDir
python -m http.server $Port
