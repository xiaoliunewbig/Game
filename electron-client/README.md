# Electron Frontend (Qt Replacement)

This directory contains the Electron-based frontend that replaces the previous Qt client UI.

## Features

- Main menu, new game, load game, and settings modals
- Gameplay screen with map grid, movement, HUD, and skill bar
- Story branches, chapter quests, companion system, and boss encounters
- Boss timeline, companion talent controls, and ending review export
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

## Verification

```bash
npm run verify
```

## Windows Packaging

```bash
npm install
npm run dist:win
```

Packaged artifacts are generated under `electron-client/dist`.

## Release Smoke Check

```bash
npm run smoke:release
```

This runs:`n- JS syntax checks (`main.js`, `preload.js`, `src/renderer.js`)`n- `npm run verify``n- Windows packaging (`npm run dist:win`)`n- Artifact checks (existence, file size, SHA256)`n- Launch probe for `dist\\win-unpacked\\*.exe``n- Report output: `dist\\artifact-report.txt`

Script path: `electron-client/scripts/release-smoke.ps1`

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

Root CMake defaults to `BUILD_QT_CLIENT=OFF`, so the legacy Qt frontend is not built unless explicitly enabled.


Optional flags for script:
- -UseNoEdit to force noedit build path
- -SkipLaunchCheck to skip process launch probe

