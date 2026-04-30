# Changelog

All notable changes to Softadastra will be documented in this file.

## 0.1.2 - Unreleased

### Added

- Added a production-oriented CLI application layer under `apps/cli`.
- Added app-level CLI commands:
  - `status`
  - `node-info`
  - `node-start`
  - `peers`
  - `store-get`
  - `store-put`
  - `sync-status`
  - `sync-tick`
- Added a composed `SoftadastraRuntime` for the CLI app.
- Added a long-running `SoftadastraNode` daemon under `apps/node`.
- Added CLI examples under `examples/cli`.
- Added new examples for discovery, metadata, store, sync, and transport workflows.

### Changed

- Improved root examples and per-module example build configuration.
- Updated app CLI command output to use structured tables and consistent terminal UI helpers.
- Improved node and runtime lifecycle code for startup, shutdown, signal handling, and deterministic ticking.
- Updated module subtrees to their latest commits.
- Updated examples to match current APIs across core, fs, wal, store, sync, discovery, metadata, and transport.

### Fixed

- Fixed app CLI command output formatting.
- Fixed timestamp rendering in app CLI commands by converting timestamps explicitly.
- Fixed `SoftadastraRuntime` initialization to use the current `CliConfig` API.
- Fixed CLI single-command startup behavior.
- Fixed node shutdown behavior with signal handlers.
- Fixed command help alignment after styled output improvements in the CLI module.

### Removed

- Removed the old root `examples/local_recovery.cpp` example.

## 0.1.1

- Restricted app builds to supported POSIX platforms.
- Updated platform module sources.
- Prepared the `v0.1.1` release.

## 0.1.0

- Initialized the Softadastra runtime.
