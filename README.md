<table>
  <tr>
    <td valign="top" width="65%">

<h1>Softadastra</h1>

<p>
  <a href="https://github.com/softadastra">
    <img src="https://img.shields.io/badge/GitHub-Follow-black?logo=github" />
  </a>
  <a href="https://github.com/softadastra/softadastra">
    <img src="https://img.shields.io/badge/C++-20-blue?logo=cplusplus" />
  </a>
  <a href="LICENSE">
    <img src="https://img.shields.io/badge/License-Apache%202.0-green" />
  </a>
</p>

<p>
  <b>The sync engine for the real world.</b>
</p>

<p>
  Softadastra is a <b>local-first, offline-capable synchronization runtime</b>
  built to keep data durable, synchronized, and recoverable under unstable networks,
  intermittent connectivity, and real-world failure conditions.
</p>

<p>
  🌍 <a href="https://softadastra.com">Website</a> ·
  📘 <a href="https://docs.softadastra.com">Docs</a> ·
  ⬇️ <a href="https://github.com/softadastra">Source</a>
</p>

</td>

<td valign="middle" width="25%" align="right">

<img
  src="https://res.cloudinary.com/dwjbed2xb/image/upload/v1767987144/android-chrome-512x512_yjmz55.png"
  width="260"
  style="border-radius:50%; object-fit:cover;"
/>

</td>
  </tr>
</table>

> A write accepted locally must remain durable, observable, and eventually synchronizable.

Softadastra is not only a set of libraries. It is a modular runtime with a command-line entry point, internal engines, and applications built on top of them.

## What Softadastra is

Softadastra is a **real synchronization system** designed around failure.

It is:

- a **local-first data runtime**
- a **durable sync engine**
- a **WAL-backed store**
- a **peer discovery and transport layer**
- a **metadata-aware node runtime**
- a **foundation for resilient applications**

A developer should not need to study every internal module before using it. The final entry point is the `softadastra` CLI, while `modules/` contains the reusable engines.

## Why Softadastra exists

Most software assumes:

- the internet is stable
- the server is always reachable
- the cloud is the source of truth
- local state is temporary
- synchronization happens only after the server accepts the write

That model fails in the real world.

Softadastra is built for:

- unstable connectivity
- intermittent networks
- offline-first environments
- edge deployments
- local-first applications
- systems that must recover after interruption

> The network is useful, but it must never be required for local correctness.

## Core model

> **Write locally. Persist first. Sync later.**

Every operation follows this model:

1. Accept the write locally
2. Persist it durably through the WAL
3. Apply it to the local store
4. Queue it for synchronization
5. Send it when peers are available
6. Retry if acknowledgement is not received
7. Converge deterministically after reconnection

The local machine remains usable even when the network is down.

## Key properties

### Local-first by design
Applications can write locally without waiting for a server or a remote peer.

### Durable by default
Accepted operations are persisted before synchronization is attempted.

### Offline-capable synchronization
Nodes can disconnect, continue working, and synchronize later.

### Deterministic convergence
When synchronization resumes, nodes converge according to deterministic conflict rules.

### Observable runtime state
The CLI exposes runtime status, node metadata, store state, sync state, and peer state.

### Modular architecture
Each internal subsystem is isolated and reusable.

### Product-level CLI
The `softadastra` binary provides a simple product-level interface over the internal runtime.

## Architecture

```text
softadastra/
├── apps/
│   ├── cli/          -> final user-facing CLI binary: softadastra
│   └── node/         -> long-running Softadastra node daemon
│
├── modules/
│   ├── core/         -> Result, Error, StrongType, IDs, time, config
│   ├── fs/           -> path, snapshot, diff, watcher
│   ├── wal/          -> write-ahead log, replay, checksum, sequence
│   ├── store/        -> WAL-backed key/value store
│   ├── sync/         -> outbox, queue, ack, retry, conflict resolution
│   ├── transport/    -> TCP transport, framing, messages, ack
│   ├── discovery/    -> peer discovery over UDP
│   ├── metadata/     -> node identity, runtime info, capabilities
│   └── cli/          -> reusable CLI engine
│
├── examples/
├── cmake/
├── data/
└── CMakeLists.txt
```

### Runtime flow

```text
Local write
  ↓
Store operation
  ↓
WAL append
  ↓
In-memory state update
  ↓
Sync outbox
  ↓
Sync queue
  ↓
Transport message
  ↓
Remote peer
  ↓
Remote store apply
  ↓
Acknowledgement
  ↓
Retry or complete
```

## Modules

### `core`
Foundational primitives used across the runtime: `Result`, `Error`, `StrongType`, `Timestamp`, IDs, config helpers, utility types.

### `fs`
Filesystem observation layer: path normalization, snapshot building, snapshot diff, polling watcher, platform watcher backends, file events.

### `wal`
Durability layer: monotonic sequence numbers, binary WAL records, checksums, append-only writer, reader, replayer.

### `store`
Local state engine: key/value operations, WAL-backed persistence, recovery from WAL, materialized in-memory state.

### `sync`
Synchronization engine: local operation submission, outbox, scheduling queue, acknowledgement tracking, retry handling, remote operation application, deterministic conflict resolution.

### `transport`
Peer-to-peer communication layer: TCP backend, length-prefixed frames, transport messages, ping/pong, sync batch messages, acknowledgements.

### `discovery`
Peer discovery layer: UDP discovery backend, announcements, probes, replies, discovered peer registry, transport integration.

### `metadata`
Node metadata layer: node identity, display name, hostname, OS name, version, capabilities, local runtime metadata.

### `cli`
Reusable command-line framework: parser, tokenizer, command registry, handlers, interactive mode, built-in commands, help formatter, table formatter, UI helpers.

## Applications

Softadastra applications live under `apps/`.

### `apps/cli`

Builds the final CLI binary: `softadastra`

The CLI runs in two modes:

- **Single command mode**
- **Interactive mode**

**Single command mode:**

```bash
softadastra status
softadastra node info
softadastra store put name gaspard
softadastra store get name
softadastra sync status
softadastra peers
```

**Interactive mode:**

```bash
softadastra
```

Example session:

```
Softadastra v0.1.0  CLI
runtime: local-first node
exit: Ctrl+D | clear: Ctrl+L | help

softadastra> help
softadastra> status
softadastra> node info
softadastra> node start
softadastra> store put name gaspard
softadastra> store get name
softadastra> sync status
softadastra> sync tick
softadastra> peers
softadastra> exit
```

### `apps/node`

Builds the long-running node daemon: `softadastra-node`

The node daemon drives: discovery polling, transport polling, sync scheduling, retry processing, peer communication.

> Use the CLI for inspection and manual operations. Use the node daemon for a long-running Softadastra node.

## CLI commands

### Help

```bash
softadastra help
```

```
softadastra

Usage
  softadastra <command> [options]

Commands
  help (h)                Show help information
  version (v)             Show CLI version
  exit (quit, q)          Exit the CLI session
  status                  Show Softadastra runtime status
  node                    Manage the local Softadastra node
  store                   Read and write local store values
  sync                    Inspect and run local sync
  peers                   List discovery and transport peers

Run 'softadastra help <command>' for details.
```

### Version

```bash
softadastra version
```

### Runtime status

```bash
softadastra status
```

Shows: node id, node running state, store entries, sync counters, transport state, discovery state, metadata state.

### Node commands

```bash
# Show node command help
softadastra node

# Show local node information
softadastra node info

# Start node services for the current CLI session
softadastra node start
```

> `softadastra node start` starts services for the current CLI process only. When the process exits, the runtime stops. For a long-running node, use `softadastra-node`.

### Store commands

```bash
# Show store command help
softadastra store

# Write a local key/value pair
softadastra store put name gaspard

# Read a local key
softadastra store get name
```

### Sync commands

```bash
# Show sync command help
softadastra sync

# Show sync state
softadastra sync status

# Run one manual sync scheduler cycle
softadastra sync tick
```

### Peer commands

```bash
# List known discovery and transport peers
softadastra peers
```

## Example CLI usage

**Start interactive mode:**

```bash
softadastra
```

Then run:

```
softadastra> status
softadastra> node info
softadastra> node start
softadastra> store put name gaspard
softadastra> store get name
softadastra> sync status
softadastra> sync tick
softadastra> peers
softadastra> exit
```

**Single commands:**

```bash
softadastra status
softadastra store put name gaspard
softadastra store get name
softadastra sync status
softadastra sync tick
```

## Build

Softadastra uses CMake internally, but the recommended way to build is through `vix build`.

```bash
vix build
```

**Build with the CLI application enabled:**

```bash
vix build -- \
  -DSOFTADASTRA_BUILD_APPS=ON \
  -DSOFTADASTRA_BUILD_CLI_APP=ON
```

**Build the node daemon too:**

```bash
vix build -- \
  -DSOFTADASTRA_BUILD_APPS=ON \
  -DSOFTADASTRA_BUILD_CLI_APP=ON \
  -DSOFTADASTRA_BUILD_NODE_APP=ON
```

**Build in release mode:**

```bash
vix build --preset release
```

**Build and export the final executable to the project root:**

```bash
vix build --bin
```

**Build only the CLI target:**

```bash
vix build --build-target softadastra
```

**Build only the node daemon target:**

```bash
vix build --build-target softadastra-node -- \
  -DSOFTADASTRA_BUILD_NODE_APP=ON
```

### Build with CMake directly

```bash
# Configure
cmake -S . -B build \
  -DSOFTADASTRA_BUILD_APPS=ON \
  -DSOFTADASTRA_BUILD_CLI_APP=ON \
  -DSOFTADASTRA_BUILD_NODE_APP=ON

# Build
cmake --build build

# Run the CLI
./build/apps/cli/softadastra

# Run the node daemon
./build/apps/node/softadastra-node
```

### Installation

```bash
cmake --install build
```

After installation:

```bash
softadastra        # CLI
softadastra-node   # daemon
```

## Current guarantees

| Guarantee | Meaning |
|---|---|
| Local writes are accepted first | The system does not depend on a remote server to accept writes |
| WAL-backed durability | Store operations can be recovered after restart |
| Retryable synchronization | Operations can be retried if acknowledgements are not received |
| Deterministic conflict resolution | Conflicts are resolved predictably |
| Network-optional behavior | The network is used for propagation, not for local correctness |
| Observable state | Runtime state can be inspected through the CLI |

## What Softadastra is not

Softadastra is not:

- a cloud storage product
- a Dropbox clone
- a Google Drive clone
- a UI application
- a database server only
- a networking library only
- a replacement for application-specific business logic

> Softadastra is the synchronization foundation underneath resilient applications.

## Design principles

- Local state is real state
- Every accepted operation must be durable
- The network is optional
- Recovery must be deterministic
- Synchronization must be observable
- Modules must stay independent
- Apps compose modules, they do not reimplement them
- The CLI must stay simple and product-friendly

## Roadmap

- [x] Core primitives
- [x] Filesystem snapshot and watcher layer
- [x] WAL writer, reader, and replay
- [x] WAL-backed store engine
- [x] Sync outbox, queue, retry, and ack tracking
- [x] TCP transport layer
- [x] UDP discovery layer
- [x] Metadata layer
- [x] CLI framework module
- [x] CLI application entry point
- [x] Grouped product CLI commands
- [x] Interactive CLI session
- [x] Long-running node daemon app
- [ ] Persistent sync outbox
- [ ] Non-blocking transport backend
- [ ] Real multi-operation sync batching
- [ ] Stronger peer identity handshake
- [ ] Encryption layer
- [ ] Cross-platform production backends
- [ ] SDK-level public API
- [ ] Documentation site

## Repository layout

```text
.
├── apps
│   ├── cli
│   └── node
├── cmake
├── data
├── examples
├── modules
│   ├── cli
│   ├── core
│   ├── discovery
│   ├── fs
│   ├── metadata
│   ├── store
│   ├── sync
│   ├── transport
│   └── wal
├── CHANGELOG.md
├── CMakeLists.txt
├── CMakePresets.json
├── LICENSE
├── README.md
└── vix.json
```

## Philosophy

> Softadastra is the Sync OS of the real internet.

The internet is not always stable.
The server is not always reachable.
The cloud should not be the only source of truth.

Softadastra treats local state as first-class and uses the network only when it is available.

> The network is unreliable. Systems should not be.

## Contributing

Good contributions improve:

- correctness under failure
- deterministic behavior
- durability
- observability
- clean module boundaries
- simple developer experience

Before adding a feature, ask:

> Does this make Softadastra more reliable under real-world failure?

## License

**Apache License 2.0**

Copyright (c) Softadastra

Licensed under the Apache License, Version 2.0.
You may not use this software except in compliance with the License.

See the [LICENSE](LICENSE) file for details.
