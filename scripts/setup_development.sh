#!/bin/bash

set -euo pipefail

# -----------------------------------------------------------------------------
# Development environment bootstrap
# -----------------------------------------------------------------------------
# This script installs core build dependencies and frontend Node dependencies.

echo "[INFO] Setting up Fantasy Legend development environment"

if [[ ! -f "CMakeLists.txt" ]]; then
    echo "[ERROR] Run this script from repository root"
    exit 1
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "[INFO] Linux detected"
    sudo apt update
    sudo apt install -y build-essential cmake git curl pkg-config
    sudo apt install -y libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc
    sudo apt install -y libpqxx-dev
    sudo apt install -y nodejs npm
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "[INFO] macOS detected"
    brew install cmake grpc protobuf libpqxx node
else
    echo "[WARN] Unsupported platform for automated setup: $OSTYPE"
    echo "[WARN] Please install dependencies manually."
fi

mkdir -p build logs temp

if [[ -d "electron-client" ]]; then
    echo "[INFO] Installing Electron frontend dependencies"
    (cd electron-client && npm install)
fi

echo "[OK] Setup complete"
echo "[INFO] Next steps:"
echo "  1) mkdir -p build && cd build && cmake .. && cmake --build . --target build_all -j"
echo "  2) ./build/bin/algorithm_server"
echo "  3) ./build/bin/strategy_server"
echo "  4) cd electron-client && npm start"
