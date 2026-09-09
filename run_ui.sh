#!/bin/bash
# DHARTI - Light Theme Web UI Launcher (macOS / Linux)
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT=8080
WEB_DIR="$DIR/web"

echo "================================================================================"
echo "   DHARTI: National Land Acquisition Control Plane (SIH 26016)                 "
echo "   Light Theme Interactive Web UI & Manual Testing Sandbox                     "
echo "================================================================================"

echo "[INFO] Serving UI from: $WEB_DIR"
echo "[INFO] Local URL: http://localhost:$PORT"

# Check available python
if command -v python3 >/dev/null 2>&1; then
    PY_BIN=python3
elif command -v python >/dev/null 2>&1; then
    PY_BIN=python
else
    echo "[ERROR] Python is required to run the local web server."
    exit 1
fi

# Open browser if on macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    open "http://localhost:$PORT" 2>/dev/null || true
elif command -v xdg-open >/dev/null 2>&1; then
    xdg-open "http://localhost:$PORT" 2>/dev/null || true
fi

cd "$WEB_DIR"
exec $PY_BIN -m http.server $PORT
