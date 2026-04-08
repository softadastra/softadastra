# Changelog

All notable changes to this project will be documented in this file.

---
## [v0.7.0]

### 🔧 Critical Fixes

- Fix umbrella package dependency resolution.
  `find_package(softadastra)` now correctly resolves all module dependencies:
  - `core`
  - `fs`
  - `wal`
  - `store`
  - `sync`
  - `transport`
  - `discovery`
  - `metadata`

- Fix CMake package lookup in installed environments.
  Properly sets `*_DIR` variables and `CMAKE_PREFIX_PATH` to ensure:
  - CI compatibility (GitHub Actions)
  - reliable external consumption

---

### 🛠 Build System

- Improve umbrella `Config.cmake` generation:
  - use `PATH_VARS` with `configure_package_config_file`
  - inject correct install paths via `PACKAGE_PREFIX_DIR`

- Stabilize multi-module package resolution:
  - all modules now discoverable from a single install prefix
  - eliminates manual `CMAKE_PREFIX_PATH` hacks

---

### 📦 Packaging

- Make umbrella package fully functional as entry point:

  ```cmake
  find_package(softadastra REQUIRED)
  ```

- Ensure transitive dependency resolution works out-of-the-box.
  Modules are now correctly chained during `find_dependency`.

---

### 🧪 CI

- Fix CI failure on smoke test (consumer project)
- Validate installation + usage in isolated environment

---

### 📚 Developer Experience

- Improved reliability when using Softadastra as external SDK
- Consistent behavior between:
  - local builds
  - CI environments
  - installed packages

---

### 🎯 Summary

This release fixes a critical issue in the umbrella CMake package and ensures that Softadastra can be reliably:

- installed
- discovered via `find_package`
- consumed as a modular C++ SDK

> Softadastra is now **production-ready** from a CMake integration perspective.

## [v0.6.0]

### 🚀 Major Improvements

- Introduce full modular CMake packaging across all modules.
  Each module (`core`, `fs`, `wal`, `store`, `sync`, `transport`, `discovery`, `metadata`, `cli`) now:
  - exports its own targets
  - provides a standalone `Config.cmake`
  - supports `find_package` usage

- Add umbrella CMake package (`softadastra`).
  Enables global integration via:

  ```cmake
  find_package(softadastra REQUIRED)
  ```

- Standardize module dependency chain. Clean and explicit architecture:

  ```
  core → fs → wal → store → sync → transport → discovery → metadata
  ```

---

### 🛠 Build System

- Add `SOFTADASTRA_WARNINGS_AS_ERRORS` global option:
  - consistent warning handling across all modules
  - compatible with GCC, Clang, and MSVC

- Improve installation layout:
  - proper use of `GNUInstallDirs`
  - headers, libraries, and CMake configs installed per module

- Remove umbrella-level header installation
  → responsibility moved to each module (clean separation)

---

### 📦 Modules

#### Core
- Add install/export support
- Improve packaging for standalone usage

#### FS
- Fix `FileState` default initialization
- Remove `-Wmissing-field-initializers` warnings
- Improve watcher reliability

#### WAL
- Add install/export support
- Improve dependency resolution

#### Store
- Add install/export support
- Align with WAL integration

#### Sync
- Add install/export support
- Improve dependency wiring (`wal` + `store`)

#### Transport
- Add install/export support
- Add platform-specific sources (Linux / macOS / Windows)

#### Discovery
- Add install/export support
- Add initial test suite

#### Metadata
- Add install/export support
- Add test suite
- Finalize top-level module integration

#### CLI
- Add install/export support
- Prepare for standalone tooling integration

---

### 🧪 Testing & CI

- Introduce GitHub Actions CI:
  - multi-platform: Linux, macOS, Windows
  - build + install + smoke tests

- Add strict CI pipeline:
  - Debug + Release matrix
  - optional examples build
  - warnings treated as errors

---

### 📚 Developer Experience

- Consistent module structure across the repository
- Improved CMake readability and maintainability
- Ready for external consumption as a C++ SDK

---

### ⚠️ Breaking Changes

- Umbrella no longer installs headers directly
  → consumers must rely on module packages (`find_package`)

---

### 🎯 Summary

This release transforms Softadastra into a **fully modular, installable, and consumable C++ SDK**, ready for:

- external integration via CMake
- CI/CD pipelines
- multi-module distribution
- future registry integration (Vix)

## [0.5.0] *(unreleased)*

### Added

- Introduce CLI module (`softadastra::cli`):
  - parser (`Tokenizer`, `CommandLine`, `ArgParser`)
  - command system (`CliCommand`, `CommandRegistry`, handlers)
  - execution engine (`CliEngine`)
  - interactive REPL support
  - structured IO (`Console`, `InputReader`, `OutputWriter`)
  - utilities (`HelpFormatter`, `TableFormatter`)
- Add CLI examples and tests
- Integrate CLI into umbrella build (optional module)

### Changed

- Update root README to position Softadastra as a sync engine (not a single product)
- Improve umbrella CMake:
  - registry-safe configuration
  - modular optional builds (`app`, `cli`)
  - installation support (headers + structure)

---

## [0.4.0]

### Added

- Modular examples support
- Improved project structure for dependency composition

### Changed

- Refactor umbrella build to be registry-safe
- Ensure compatibility with Vix:
  - `vix install`
  - `vix build`
  - `vix run`

---

## [0.3.0]

### Changed

- Update `store`, `sync`, and `transport` modules to latest versions
- Improve synchronization pipeline stability

### Notes

- System closer to real-world sync scenarios

---

## [0.2.0]

### Added

- Initial local-first pipeline:

  ```text
  fs → WAL → store → recovery
  ```

- Durable write model
- Recovery mechanism after restart

### Notes

- First working proof of local-first architecture

---

## [0.1.0]

### Added

- Initial modular architecture:
  - `core`
  - `fs`
  - `wal`
  - `store`
  - `sync`
  - `transport`
  - `discovery`

### Changed

- Restructure project into modular system

---

## Philosophy

Softadastra evolves as a sync engine for real-world systems:

- **local-first**
- **offline-capable**
- **deterministic**
- **resilient under failure**
