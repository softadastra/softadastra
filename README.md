# Softadastra Drive

> Local-first file sync that works even when the internet doesn’t.

Softadastra Drive is a minimal, high-reliability synchronization engine that proves a simple idea:

> A file modified on one machine appears on another
> even if the internet is down.

---

## Why Softadastra Drive exists

Modern applications assume:

* Stable internet
* Always-available servers
* Cloud as the source of truth

That assumption breaks in the real world.

Softadastra Drive is built on a different model:

* Local is the source of truth
* The network is optional
* Sync is eventual, deterministic, and resilient

---

## What this project is

Softadastra Drive is **not** a cloud storage product.

It is:

* A **local-first sync engine**
* A **proof of reliable synchronization without cloud dependency**
* A **foundation for building offline-capable applications**

---

## Core idea

> Write locally. Persist first. Sync later.

Every operation:

1. Is accepted locally
2. Is persisted durably (WAL)
3. Is synchronized when possible

No operation depends on the network to succeed.

---

## Key Features

### Local-first writes

All file operations are accepted instantly without waiting for network confirmation.

### Durable WAL (Write-Ahead Log)

Every change is recorded before any network activity.

### LAN-based sync

Devices discover each other and synchronize over the local network.

### Recovery after interruption

If a device goes offline, it catches up when it reconnects.

### Deterministic convergence

All peers eventually reach the same state.

---

## What this is NOT

* Not Google Drive
* Not Dropbox
* Not a cloud storage clone
* Not a collaboration suite
* Not a file explorer UI

This is the **sync engine** behind future applications.

---

## Demo scenario

This is the only thing that must work:

1. Start Softadastra Drive on two machines (same LAN)
2. Create or modify a file on Machine A
3. Machine B receives the update
4. Disconnect internet (keep LAN)
5. Modify the file again on A
6. B still receives the update
7. Turn B off, modify files on A
8. Turn B back on → it catches up

If this works, the system is valid.

---

## Architecture (MVP)

The system is intentionally minimal.

### 1. Watched Folder

A local directory monitored for changes.

### 2. Local Metadata Store

Tracks file state, versions, and sync status (SQLite).

### 3. WAL (Write-Ahead Log)

Persists all operations before sync.

### 4. Peer Discovery

Detects other devices on the LAN.

### 5. Sync Engine

Transfers missing operations between peers.

### 6. Replay / Recovery

Rebuilds state after disconnection.

---

## Guarantees

Softadastra Drive is built around strict invariants:

* No accepted write is ever lost
* Every operation is persisted before sync
* Offline peers do not break system consistency
* All peers converge after reconnection
* Cloud is never required

---

## Scope (MVP)

* 2 devices
* 1 shared folder
* LAN only
* Basic file operations
* No complex conflict resolution

---

## Philosophy

Softadastra is the **Sync OS of the real internet**.

This means:

* Systems must work under failure
* Connectivity is unreliable by default
* The network is an optimization, not a dependency

---

## Installation (coming soon)

```bash
# example (future CLI)
vix add @softadastra/drive
## vix run drive --folder ~/SoftadastraDrive
```

---

## Roadmap

* Multi-device sync
* Delta-based transfer
* Conflict resolution strategies
* Encryption layer
* SDK integration
* Cross-platform support

---

## Vision

Softadastra Drive is not the product.

It is the **proof**.

From this proof will emerge:

* SDKs
* Platforms
* Offline-first applications
* A new standard for synchronization

---

## Contributing

This project is early and focused.

Contributions should prioritize:

* Correctness under failure
* Simplicity over features
* Deterministic behavior

---

## License

TBD

---

## Final note

> Softadastra Drive is not trying to replace the cloud.
> It is proving that the cloud is optional.
