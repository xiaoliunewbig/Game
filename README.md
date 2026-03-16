# Fantasy Legend

Fantasy Legend is a 2D action RPG framework with gRPC backend services and an Electron desktop frontend.

## Active Stack

- Backend: C++20 (`algorithm/`, `strategy/`)
- RPC: gRPC + Protobuf (`proto/`)
- Frontend: Electron (`electron-client/`)

`application/` exists only as archived legacy source and is not used in the active build flow.

## Build Backend

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target build_all -j
```

### Windows Quick Build

```powershell
cd D:\Game\Game
.\scripts\build_windows.ps1 -BuildDir build-vs2022 -BuildType Release
```

If you use Conda and hit MSBuild `Path/PATH` environment conflicts, run `conda deactivate` and open a fresh terminal before building.

## Run Backend Services

```bash
# Terminal 1
./build/bin/algorithm_server

# Terminal 2
./build/bin/strategy_server
```

## Run Electron Frontend

```bash
cd electron-client
npm install
npm start
```

## Main Ports

- Algorithm Service: `50051`
- Strategy Service: `50052`

## Repository Layout

```text
algorithm/          Algorithm gRPC service
strategy/           Strategy gRPC service
electron-client/    Electron frontend
application/        Archived legacy frontend
proto/              Protobuf definitions
tests/              Test targets
scripts/            Build and utility scripts
```
