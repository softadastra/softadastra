# 📦 CHANGELOG — drive-core

All notable changes to this project will be documented in this file.

The format is based on **Keep a Changelog**
👉 https://keepachangelog.com/en/1.0.0/
and this project adheres to **Semantic Versioning**
👉 https://semver.org/

---

## 🚧 [Unreleased]

Planned features for upcoming versions:

### Authentication

- User signup, login, logout
- Refresh tokens + device sessions
- Password hashing and validation

### Files & Folders

- File upload/download/delete
- Versioning & metadata
- Folder CRUD and tree API
- Breadcrumb API

### Sync Engine

- File index tracking
- Change cursors
- Conflict rules
- Real-time updates through WebSocket

### AI Integration

- Document summarization
- OCR for images/PDFs
- Auto-tags generation
- Semantic search

### Backend Foundation

- Central configuration system
- Structured logging
- MySQL connector integration
- Error handling middleware

### Testing

- Service-level unit tests (Auth, Files, Sync)
- Route tests
- CI workflows (optional future)

---

## 🟦 [0.1.0] — Initial Project Skeleton

**Date:** YYYY-MM-DD

### Added

- Initial project structure for `drive-core`
- Namespace structure: `include/softadastra/drive/...`
- `App` class using the PIMPL pattern
- Basic HTTP routes:
  - `GET /` (service status)
  - `GET /health` (health check)
- Integration with **Vix.cpp** runtime
- CMake build system:
  - Static library `softadastra_drive_core`
  - Executable `drive-core`
  - Support for sanitizers (ASan/UBSan)
  - Boost, Threads, and OpenSSL integration
- Test environment:
  - Enabled CTest
  - Added Catch2 via FetchContent
  - Basic test executable `drive-core-tests`
- `run` CMake custom target for quick development

### Changed

- Improved include structure for future modular expansion
- Prepared internal folder layout for controllers, services, repositories, utils, and WebSocket modules

### Fixed

- JSON ambiguity resolved using explicit `nlohmann::json`
- Improved compatibility with both `vix::vix` and `Vix::vix` namespaces

---

## 📌 History

This changelog begins with version **0.1.0**.
