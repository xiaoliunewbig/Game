# Electron Frontend (Qt Replacement)

This directory contains a full Electron-based frontend that replaces the previous Qt client UI.

## Features

- Main menu, new game, load game, and settings modals
- Gameplay screen with map grid, movement, HUD, and skill bar
- Inventory drawer UI
- gRPC bridge to:
  - Algorithm Service (`localhost:50051`)
  - Strategy Service (`localhost:50052`)

## Quick Start

```bash
cd electron-client
npm install
npm start
```

## Development Mode

```bash
npm run start:dev
```

This opens Chromium DevTools automatically.

## gRPC API Mapping

- `service:health` -> `ValidateInput`, `QueryGameState`
- `algorithm:calculate-damage` -> `CalculateDamage`
- `algorithm:skill-tree` -> `GetSkillTree`
- `algorithm:ai-decision` -> `AIActionDecision`
- `strategy:game-rules` -> `GetGameRules`
- `strategy:update-world` -> `UpdateWorldState`
- `strategy:trigger-event` -> `TriggerEvent`
- `strategy:query-state` -> `QueryGameState`

## Build Note

Root CMake now defaults to `BUILD_QT_CLIENT=OFF`, so the legacy Qt frontend is not built unless explicitly enabled.
