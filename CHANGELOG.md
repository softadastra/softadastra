# Changelog

All notable changes to this project will be documented in this file.

---

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
