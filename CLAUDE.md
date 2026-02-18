# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

幻境传说 (Fantasy Legend) — a 2D action RPG game framework built with C++20, Qt 6.3+, and gRPC microservices. The project uses a three-layer architecture where services communicate over gRPC.

## Build Commands

The project uses CMake 3.16+ as its build system.

```bash
# Full build (from project root)
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make build_all -j$(nproc)

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON ..

# Build a single target
make game_client          # Qt game client only
make algorithm_service    # Algorithm service only
make strategy_service     # Strategy service only

# Run tests
cd build && ctest --verbose
# or: make test

# Using the build script (Linux/macOS)
./scripts/build.sh                        # Release build
./scripts/build.sh -t Debug -c            # Clean debug build
./scripts/build.sh --target game_client   # Single target
./scripts/build.sh --test                 # Build and run tests

# Generate docs (requires Doxygen)
make docs
```

Build outputs go to `build/bin/` (executables), `build/lib/` (libraries), `build/tests/` (test binaries).

**Dependencies:** Qt6 (Core, Quick, Widgets, Multimedia, Network), gRPC, Protobuf, Threads, PostgreSQL (libpqxx for strategy service).

## Architecture

Three independent layers communicating via gRPC:

```
Algorithm Service (port 50051)  ← Combat calc, AI decisions, skill trees
Strategy Service  (port 50052)  ← Game rules, world state, player persistence
Qt Application    (port 8080)   ← Game client, rendering, UI
```

### Algorithm Layer (`algorithm/`)
Static library `algorithm_service`. Core classes: `AlgorithmService`, `DamageCalculator`, `AIDecisionEngine`, `SkillTreeManager`. Pure computation with no database dependencies.

### Strategy Layer (`strategy/`)
Static library `strategy_service`. Organized into subsystems:
- `Algorithm_interact/` — service coordination: `StrategyService`, `GameRuleManager`, `WorldStateEngine`, `EventScheduler`
- `Log/` — async logging: `AsyncLogService`, `LogServiceConsole`, `LogServiceFile`
- `player_service/` — player persistence: `PlayerService`, `PlayerRepositoryPostgres`
- `database/` — PostgreSQL: `PostgreSQLConnection`, `DatabaseConfig`, `BaseRepository`
- `monitor/` — performance monitoring

### Application Layer (`application/`)
Qt6 executable `game_client`. Uses both QML (Quick) and Widgets. Subsystem layout under `include/` and `src/`:
- `core/` — `GameEngine` (main loop, state machine), `SceneManager`, `ResourceManager`, `EventSystem`
- `game/` — `BattleSystem` (turn-based), `Player`, `InventorySystem`, `GameState`
- `network/` — `NetworkManager`, `StrategyServiceClient`, `AlgorithmServiceClient`
- `graphics/` — `SpriteRenderer`, `AnimationManager`
- `input/` — `InputManager`
- `audio/` — `AudioManager`
- `save/` — `SaveManager`
- `ui/` — Window classes for each screen
- `qml/` — QML UI files and components

`GameEngine` runs at 60 FPS target and manages states: Uninitialized, MainMenu, Loading, Playing, Battle, Paused, GameOver.

### Proto Definitions (`proto/`)
- `AlgorithmService.proto` — `CalculateDamage`, `GetSkillTree`, `AIActionDecision`, `ValidateInput`
- `StrategyService.proto` — `GetGameRules`, `UpdateWorldState`, `TriggerEvent`, `QueryGameState`

### Tests (`tests/`)
Uses CTest. Currently has `strategy_log_test/`. Test framework is Google Test.

## Code Conventions

- **C++20** standard, warnings as errors (`-Werror` / `/WX`)
- **No `using namespace std`**; use explicit namespace qualifiers
- **Smart pointers only** — no raw pointer resource management, no manual `new`/`delete`
- Member variables use trailing underscore: `health_`
- No public member variables; use getters/setters
- Constructors marked `explicit`
- Header guards: `#ifndef MODULE_CLASSNAME_H`
- Namespaces per module (e.g., `namespace game_core { }`)
- 4-space indentation
- No magic numbers; constants defined centrally
- Headers include only necessary dependencies; use forward declarations
- Doxygen-style comments (`@brief`, `@param`, `@return`) for classes and functions
- Qt signals/slots and Q_PROPERTY for QML integration
- MSVC uses `/utf-8` encoding flag

## Compiler Flags

**MSVC:** `/W4 /WX /utf-8 /permissive-`
**GCC/Clang:** `-Wall -Wextra -Wpedantic -Werror -fPIC`
