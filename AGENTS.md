# AGENTS.md

Guidance for coding agents in this repository.

## Overview

Fantasy Legend currently uses:
- C++20 backend services (`algorithm/`, `strategy/`)
- gRPC contracts (`proto/`)
- Electron frontend (`electron-client/`)

`application/` is archived legacy code and should not be used for new implementation work.

## Build and Run

### Backend

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target build_all -j
ctest --verbose
```

### Frontend

```bash
cd electron-client
npm install
npm start
```

## Coding Rules

- Use C++20 for backend code
- Treat warnings as errors
- Avoid `using namespace std`
- Prefer smart pointers
- Keep 4-space indentation
- Use Doxygen comments for backend APIs

## Frontend Rules

- Use Electron preload + IPC for privileged operations
- Keep Node access out of renderer code
- Keep renderer state changes isolated from service transport logic

## Documentation Rule

When updating docs or scripts, reference Electron as the primary frontend.
