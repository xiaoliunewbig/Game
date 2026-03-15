#!/bin/bash

set -euo pipefail

# -----------------------------------------------------------------------------
# Fantasy Legend build helper
# -----------------------------------------------------------------------------
# Builds backend services via CMake.
# Electron frontend is handled in electron-client/ with npm.

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info() { echo -e "${BLUE}[INFO]${NC} $1"; }
success() { echo -e "${GREEN}[OK]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERR]${NC} $1"; }

BUILD_TYPE="Release"
TARGET="build_all"
CLEAN="false"
RUN_TESTS="false"

usage() {
    cat <<EOF
Usage: $0 [options]

Options:
  -t, --type <Debug|Release|RelWithDebInfo>  Build type (default: Release)
  --target <name>                             CMake target (default: build_all)
  -c, --clean                                 Remove build directory before configure
  --test                                      Run ctest after build
  -h, --help                                  Show help

Examples:
  $0
  $0 -t Debug --test
  $0 --target strategy_server
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --target)
            TARGET="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN="true"
            shift
            ;;
        --test)
            RUN_TESTS="true"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

if [[ ! -f "CMakeLists.txt" ]]; then
    error "Run this script from repository root"
    exit 1
fi

if [[ "$CLEAN" == "true" ]]; then
    info "Cleaning build directory"
    rm -rf build
fi

mkdir -p build
cd build

info "Configuring CMake (type=${BUILD_TYPE})"
cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..

info "Building target: ${TARGET}"
cmake --build . --target "${TARGET}" -j

if [[ "$RUN_TESTS" == "true" ]]; then
    info "Running tests"
    ctest --verbose
fi

success "Build completed"

echo
info "Typical run commands:"
echo "  ./build/bin/algorithm_server"
echo "  ./build/bin/strategy_server"
echo "  cd electron-client && npm install && npm start"
