#!/usr/bin/env bash

set -euo pipefail

BUILD_DIR="build"
BUILD_TYPE="Release"
GENERATOR="Ninja"
CLEAN="false"
SKIP_CONFIGURE="false"
SKIP_BUILD="false"
SKIP_TESTS="false"
RUN_BACKEND="true"
RUN_FRONTEND="true"

usage() {
  cat <<EOF
Usage: $0 [options]

Options:
  --build-dir <dir>                 Build directory (default: build)
  --build-type <type>               CMake build type (default: Release)
  --generator <name>                CMake generator (default: Ninja)
  --clean                           Remove build directory before configure
  --backend-only                    Run backend configure/build/test only
  --frontend-only                   Run frontend npm verify only
  --skip-configure                  Skip backend configure step
  --skip-build                      Skip backend build step
  --skip-tests                      Skip backend ctest step
  -h, --help                        Show this help

Examples:
  $0
  $0 --backend-only --build-dir build
  $0 --frontend-only
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --build-type)
      BUILD_TYPE="$2"
      shift 2
      ;;
    --generator)
      GENERATOR="$2"
      shift 2
      ;;
    --clean)
      CLEAN="true"
      shift
      ;;
    --backend-only)
      RUN_BACKEND="true"
      RUN_FRONTEND="false"
      shift
      ;;
    --frontend-only)
      RUN_BACKEND="false"
      RUN_FRONTEND="true"
      shift
      ;;
    --skip-configure)
      SKIP_CONFIGURE="true"
      shift
      ;;
    --skip-build)
      SKIP_BUILD="true"
      shift
      ;;
    --skip-tests)
      SKIP_TESTS="true"
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "[ERR] Unknown option: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ ! -f "CMakeLists.txt" ]]; then
  echo "[ERR] Run this script from repository root" >&2
  exit 1
fi

if [[ "$RUN_BACKEND" == "true" ]]; then
  if [[ "$CLEAN" == "true" ]]; then
    rm -rf "$BUILD_DIR"
  fi

  if [[ "$SKIP_CONFIGURE" == "false" ]]; then
    cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  fi

  if [[ "$SKIP_BUILD" == "false" ]]; then
    cmake --build "$BUILD_DIR" --parallel
  fi

  if [[ "$SKIP_TESTS" == "false" ]]; then
    ctest --test-dir "$BUILD_DIR" --output-on-failure --parallel 4
  fi
fi

if [[ "$RUN_FRONTEND" == "true" ]]; then
  if [[ ! -d "electron-client" ]]; then
    echo "[ERR] Missing electron-client directory" >&2
    exit 1
  fi

  pushd electron-client >/dev/null
  npm install
  npm run verify
  popd >/dev/null
fi

echo "[OK] Full verification completed."
