# Changelog

All notable changes to **Softadastra Drive** will be documented in this file.

This project follows a structured release approach focused on **reliability, local-first guarantees, and deterministic sync behavior**.

---

## [0.1.0] - Initial MVP Release

### Added

* Local-first folder synchronization between two devices on the same LAN
* Watched folder system for detecting file changes (create, update, delete)
* Local metadata store (SQLite) for tracking file state, versions, and sync status
* Write-Ahead Log (WAL) for durable operation persistence before network sync
* Basic peer discovery mechanism over local network (LAN only)
* Initial file transfer system (full file replication on change)
* Replay and recovery system for resuming sync after peer disconnection
* Minimal CLI / local runtime to manage sync session
* Status indicators: `synced`, `pending`, `offline`

### Guarantees

* Local writes are accepted without requiring network connectivity
* All operations are persisted before being sent over the network
* No data loss for accepted operations
* Deterministic convergence between peers after reconnection
* No dependency on cloud infrastructure

---

## [0.1.1] - Stability Improvements

### Improved

* Better handling of rapid file changes (debounce and batching)
* Reduced duplicate sync operations during high-frequency updates
* Improved WAL replay consistency on restart
* More robust peer reconnection logic

### Fixed

* Edge cases where deleted files could reappear after reconnection
* Inconsistent state when peer disconnects during transfer
* Minor metadata inconsistencies in SQLite store

---

## [0.2.0] - Sync Reliability Upgrade

### Added

* Incremental sync tracking (operation-based instead of full state scan)
* Sync queue system for ordered and reliable operation delivery
* Basic conflict avoidance strategy (last-write-wins for MVP scope)
* File integrity verification using hashing

### Improved

* Faster recovery after peer reconnection
* Reduced unnecessary file transfers
* More accurate sync state reporting

### Fixed

* Race conditions between file watcher and WAL logging
* Partial transfer issues during network interruptions

---

## [0.3.0] - Network Layer Enhancements

### Added

* Improved LAN peer discovery (broadcast + retry strategy)
* Connection health monitoring (heartbeat / ping system)
* Retry mechanism with backoff for failed transfers

### Improved

* Stability of connections on unstable local networks
* Better handling of temporary network partitions
* Reduced sync latency on small file updates

---

## [0.4.0] - Developer Experience

### Added

* Structured logs (JSON mode for debugging and observability)
* CLI improvements for starting and inspecting sync sessions
* Debug tools for inspecting WAL and metadata state

### Improved

* Clearer runtime output for sync status
* Easier troubleshooting of sync issues

---

## [0.5.0] - Pre-SDK Foundation

### Added

* Internal modularization of sync engine
* Clear separation between:

  * Storage layer
  * Sync engine
  * Network transport
* Preparation for future SDK integration

### Improved

* Codebase organization for extensibility
* Internal APIs for future embedding in external applications

---

## Roadmap (Next)

### Planned

* Multi-peer synchronization (more than 2 devices)
* Partial file transfer (true delta sync)
* Advanced conflict resolution strategies
* Optional encryption layer
* Mobile and cross-platform support
* Cloud-assisted relay (optional, not required)

---

## Philosophy

Softadastra Drive is not a cloud storage product.

It is:

> A proof that reliable synchronization can exist without requiring the cloud.

---

## Notes

* This project prioritizes **correctness under failure** over features
* Network is considered an **optimization**, not a requirement
* All guarantees are built around **local durability and convergence**
