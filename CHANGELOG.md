# Changelog

All notable changes to this project will be documented in this file.

---
## [v0.10.0]

### Improvements

- add command options support (`CliOption`)
- improve help system:
  - `help` lists commands dynamically
  - `help <command>` shows detailed command usage
  - support `--help` on commands
- remove hardcoded logic for `help` and `version` (now context-driven)
- improve input handling with `std::optional` (EOF-safe)
- stabilize interactive CLI loop

### Fixes

- fix missing CLI symbols (InputReader implementation)
- ensure CLI module is correctly integrated in umbrella builds


## [v0.9.5]

### Breaking Changes

- remove app module from core
  The `modules/app` submodule has been removed from the Softadastra core repository.
  Softadastra is now strictly a foundation layer and no longer includes any application entry point.

- remove drive-cli from core
  The `apps/drive-cli` submodule has been removed.
  Product-level implementations are now separated from the core infrastructure.

### Improvements

- clarify architecture boundaries
  Softadastra is now fully defined as a pure infrastructure layer:
  - core modules (wal, sync, transport, etc.)
  - reusable CLI engine
  - no product logic

- improve repository structure
  The repository now focuses exclusively on:
  - synchronization engine
  - offline-first primitives
  - developer-facing modules

- align with platform vision
  This change prepares Softadastra to act as a foundation similar to:
  - Stripe (payments infrastructure)
  - Firebase (backend platform)
  - Supabase (data infrastructure)

  Products such as Softadastra Drive will live in separate repositories.

### Internal

- clean submodule configuration
  Removed unused submodules and simplified `.gitmodules`.

- reduce conceptual complexity
  Eliminated ambiguity between infrastructure and product layers.


## [v0.9.4]

### Fixes

- fix(wal): include missing `<cstddef>` for `std::size_t`
  Resolves compilation errors on AppleClang and other strict toolchains where `std::size_t` was not available through `<cstdint>`.

- fix(wal): restore cross-platform build compatibility
  Ensures WAL encoding/decoding components compile correctly on macOS, Linux, and Windows.

### Improvements

- improve portability of WAL module
  Aligns header includes with standard C++ requirements for better compiler compatibility.

### Internal

- refactor(wal): clarify header dependencies
  Explicitly include required standard headers instead of relying on indirect includes.

---

Softadastra v0.9.4 fixes a critical cross-platform compilation issue and improves standard compliance of the WAL module.

## [v0.9.3]

### Improvements

- improve CI reliability with submodule support
  CI workflows now fetch all required submodules recursively, ensuring full project structure is available during builds.

- improve build consistency
  Umbrella build now correctly includes all modules (`core`, `fs`, `wal`, `store`, `sync`, etc.) in CI environments.

- improve cross-environment reproducibility
  Aligns CI behavior with local development by ensuring modules are always present during configuration.

### Fixes

- fix(ci): resolve missing module targets in CI
  Fixes failures where modules were skipped (`missing CMakeLists.txt`) due to submodules not being fetched.

- fix(ci): restore correct target resolution in examples
  Fixes errors like `softadastra::core not found` during example builds and smoke tests.

### Internal

- refactor(ci): enable recursive submodule checkout
  Updated GitHub Actions checkout step to include:
  `submodules: recursive`

---

Softadastra v0.9.3 fixes a critical CI issue and ensures all modules are correctly included during automated builds.


## [v0.9.2]

### Improvements

- improve CI package consistency
  Build and install now use a unified install prefix, ensuring consistent package layout across all environments.

- improve cross-platform reliability
  Eliminates discrepancies between configure-time and install-time paths in CI workflows.

- improve developer experience
  Ensures `find_package(softadastra CONFIG)` works reliably in automated pipelines and real consumer scenarios.

### Fixes

- fix(ci): resolve missing module config files in smoke tests
  Fixes failures where installed module configs (`softadastra_core`, `fs`, `wal`, etc.) could not be found during CI runs.

- fix(ci): align configure and install prefixes
  Prevents mismatch between generated package paths and actual install locations.

### Internal

- refactor(ci): simplify install step
  Removed redundant `--prefix` override and rely on a single source of truth (`CMAKE_INSTALL_PREFIX`).

---

Softadastra v0.9.2 stabilizes CI behavior and guarantees reliable package consumption across all platforms.

## [v0.9.1]

### Improvements

- improve CI reliability
  All workflows now use a clean build, install, and smoke environment to ensure deterministic results.

- improve cross-platform stability
  Consistent behavior across Linux, macOS, and Windows by eliminating stale build artifacts.

- improve developer experience
  Smoke tests now accurately reflect real consumer usage of installed Softadastra packages.

### Fixes

- fix(ci): remove stale build and install artifacts
  Fixes intermittent failures caused by leftover `build/`, `install/`, and `smoke/` directories in CI.

- fix(ci): ensure correct package resolution during smoke tests
  Prevents false negatives where `find_package(softadastra)` failed despite correct installation.

- fix(ci): avoid missing CLI package errors
  Disables CLI module in CI workflows when not installed, preventing invalid includes in umbrella config.

### Internal

- refactor(ci): unify workflow behavior across all pipelines
  Both standard and strict CI workflows now follow the same clean setup strategy.

---

Softadastra v0.9.1 ensures that CI results are **reliable, reproducible, and representative of real-world usage**.

## [v0.9.0]

### Improvements

- improve CMake package correctness
  Fixed path resolution in the umbrella package to ensure correct loading of module configurations across all platforms.

- improve installation portability
  The umbrella config now resolves installed module paths reliably without duplicating the install prefix.

- improve cross-platform compatibility
  Consistent behavior for package loading on Linux, macOS, and Windows.

- improve developer experience
  `find_package(softadastra CONFIG)` now works out-of-the-box without requiring manual path fixes.

### Fixes

- fix(cmake): resolve invalid include paths in umbrella config
  Fixes duplicated install prefix issue that caused invalid paths such as:
  `install//install/lib/cmake/...`

- fix(cmake): restore proper module config loading
  Ensures all module configs (`core`, `fs`, `wal`, `store`, `sync`, etc.) are correctly included after installation.

### Internal

- refactor(cmake): simplify path resolution in config template
  Removed incorrect prefix concatenation using `PACKAGE_PREFIX_DIR`.

---

Softadastra v0.9.0 stabilizes the CMake ecosystem and ensures reliable package usage in real-world environments.

## [v0.8.0]

### Improvements

- improve CMake umbrella reliability
  The umbrella package now loads module configurations directly from the install prefix instead of relying on `find_dependency()`.
  This ensures deterministic behavior across platforms (Linux, macOS, Windows).

- improve package resolution stability
  Eliminates dependency lookup issues during `find_package(softadastra CONFIG)` in CI and external projects.

- improve cross-platform consistency
  Unified behavior for package discovery across AppleClang, GCC, and MSVC environments.

- improve developer experience
  Simplified integration: consumers no longer need to configure `CMAKE_PREFIX_PATH` manually for submodules.

### Fixes

- fix(cmake): resolve missing `softadastra_core` package error
  Fixes failure where CMake could not locate module configs (`softadastra_coreConfig.cmake`, etc.) during umbrella usage.

- fix(cmake): eliminate fragile dependency resolution
  Removes indirect dependency resolution via `find_dependency()` which caused inconsistent results depending on environment.

### Internal

- refactor(cmake): switch to direct inclusion of module config files
  The umbrella now explicitly includes installed module configs from:
  `lib/cmake/softadastra_*`

- refactor(cmake): simplify CLI module integration
  CLI config is now conditionally included using the same deterministic mechanism.

---

Softadastra v0.8.0 marks a key milestone toward a **fully deterministic, installable, and portable CMake ecosystem**.

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
