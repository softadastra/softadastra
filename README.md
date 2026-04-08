# Softadastra

> **The sync engine for the real world.**

Softadastra is a **local-first, offline-capable synchronization system** designed to work reliably under real network conditions.

It guarantees a simple outcome:

> Data written on one machine becomes available on another,
> even when the internet is unstable or unavailable.

---

## What Softadastra is

Softadastra is a **real system**, not a concept. It is:

- a **synchronization engine**
- a **local-first data layer**
- a **foundation for building resilient applications**

It works across machines, over unreliable networks, without depending on the cloud.

---

## Why Softadastra exists

Most systems assume:

- stable internet
- always-available servers
- cloud as the source of truth

This assumption fails in the real world.

Softadastra is built for:

- unstable connectivity
- intermittent networks
- offline-first environments

---

## Core model

> *Write locally. Persist first. Sync later.*

Every operation:

1. is accepted **locally**
2. is persisted **durably** (WAL)
3. is synchronized **when possible**

The network is **never required for correctness**.

---

## What makes it different

### Local-first by design

All writes succeed instantly without waiting for the network.

### Durable by default

Every operation is persisted before any sync attempt.

### Network-optional synchronization

Devices synchronize when connectivity is available, but never depend on it.

### Resilient to failure

Nodes can disconnect, restart, or fail without breaking the system.

### Deterministic convergence

All peers converge to the same state after synchronization.

---

## What this is NOT

- not a cloud storage product
- not Google Drive or Dropbox
- not a UI or application

Softadastra is the **engine underneath applications**.

---

## Real-world behavior

Softadastra is designed for real conditions:

1. Two machines start on the same network
2. Data is written on Machine A
3. Machine B receives updates
4. Network becomes unstable or disconnected
5. Writes continue locally on each node
6. Synchronization resumes when connectivity is restored
7. System converges automatically

---

## Architecture

```
Local state (store)    →  source of truth
WAL                    →  durability layer
Sync engine            →  propagates operations
Transport layer        →  network abstraction (LAN, future P2P)
Discovery              →  finds peers
Recovery engine        →  rebuilds state after interruptions
```

---

## Guarantees

Softadastra is built on strict invariants:

| Guarantee | Description |
|-----------|-------------|
| **No write is ever lost** | Every accepted write is persisted before sync |
| **Offline safety** | Offline nodes do not break consistency |
| **Convergence** | All peers converge after reconnection |
| **Network-optional** | The network is never required for correctness |

---

## Philosophy

Softadastra is the **Sync OS of the real internet**.

- local is the source of truth
- the network is an optimization
- systems must work under failure

---

## Installation

```bash
vix add @softadastra/sync
```

> Full installation guide coming soon.

---

## Roadmap

- [ ] Multi-device synchronization
- [ ] Delta-based replication
- [ ] Conflict resolution strategies
- [ ] Encryption layer
- [ ] SDK and APIs
- [ ] Cross-platform support

---

## Vision

Softadastra is not an application. It is a **foundational layer** — a system that enables:

- offline-first applications
- resilient infrastructure
- decentralized data systems

---

## Contributing

Focus areas:

- **correctness** under failure
- **deterministic** behavior
- **simplicity**

---

## License

MIT

---

> *The network is unreliable. Systems should not be.*
