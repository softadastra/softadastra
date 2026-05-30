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

Softadastra is a modular synchronization runtime with a product-level CLI, a long-running node daemon, and reusable C++ engines for durability, storage, synchronization, transport, discovery, and metadata.

## What Softadastra is

Softadastra is a synchronization foundation for software that must continue working when the network is slow, unstable, or unavailable.

It provides:

- a local-first runtime model
- a WAL-backed durability layer
- a recoverable key-value store
- a sync engine with queue, retry, ack, and conflict handling
- peer transport and local discovery
- node metadata and observability
- a product-level CLI entry point

A developer should not need to understand every internal module before using Softadastra. The main entry point is the `softadastra` CLI. The `modules/` directory contains the reusable engines that power it.

## Why Softadastra exists

Most software assumes the server is reachable, the network is stable, and the cloud is the source of truth.

That model breaks in real environments.

Softadastra is built for systems where:

- users may go offline
- networks may be unstable
- writes must not be lost
- synchronization may happen later
- recovery must be deterministic
- local state must remain useful

> The network is useful, but it must never be required for local correctness.

## Core model

> Write locally. Persist first. Sync later.

Every accepted operation follows this flow:

```text
Local write
  ↓
WAL append
  ↓
Local store apply
  ↓
Sync outbox
  ↓
Transport message
  ↓
Remote peer
  ↓
Ack / retry / converge
```

The local machine remains usable even when the network is down.

## Applications

Softadastra exposes two main applications.

### `softadastra`

The product-level CLI for developers, operators, scripts, and local inspection.

```bash
softadastra status
softadastra node info
softadastra store put name gaspard
softadastra store get name
softadastra sync status
softadastra peers
```

Interactive mode:

```bash
softadastra
```

Example session:

```text
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

### `softadastra-node`

The long-running node daemon.

It drives:

- discovery polling
- transport polling
- sync scheduling
- retry processing
- peer communication
- background runtime services

Use `softadastra` for inspection and manual operations. Use `softadastra-node` for a long-running local node.

## Architecture

```text
softadastra/
├── apps/
│   ├── cli/          -> final user-facing CLI binary: softadastra
│   └── node/         -> long-running Softadastra node daemon
│
├── modules/
│   ├── core/         -> foundational primitives
│   ├── fs/           -> filesystem observation
│   ├── wal/          -> write-ahead log and replay
│   ├── store/        -> WAL-backed local state
│   ├── sync/         -> local-first sync orchestration
│   ├── transport/    -> peer message delivery
│   ├── discovery/    -> local peer discovery
│   ├── metadata/     -> node identity and capabilities
│   └── cli/          -> reusable CLI framework
│
├── examples/
├── cmake/
├── data/
└── CMakeLists.txt
```

## Modules

| Module      | Role                                                                        |
| ----------- | --------------------------------------------------------------------------- |
| `core`      | Foundational primitives: result types, errors, IDs, time, config, utilities |
| `fs`        | Filesystem observation: paths, snapshots, diffs, watchers, file events      |
| `wal`       | Durability: append-only records, sequence numbers, checksums, replay        |
| `store`     | WAL-backed key-value state with deterministic recovery                      |
| `sync`      | Outbox, queue, retry, ack tracking, remote apply, conflict policy           |
| `transport` | Peer message delivery, framing, TCP backend, ping/pong, sync batches        |
| `discovery` | Local peer discovery through announcements, probes, replies, registry       |
| `metadata`  | Node identity, runtime information, version, platform, capabilities         |
| `cli`       | Command parser, registry, handlers, help output, interactive sessions       |

Each module has its own README. The full documentation lives at [docs.softadastra.com](https://docs.softadastra.com).

## Current guarantees

| Guarantee                 | Meaning                                                                   |
| ------------------------- | ------------------------------------------------------------------------- |
| Local writes first        | A write does not require a remote server to be accepted locally           |
| WAL-backed durability     | Accepted operations can be recovered after restart                        |
| Retryable sync            | Operations can be retried until acknowledged                              |
| Deterministic recovery    | State can be rebuilt from durable records                                 |
| Network-optional behavior | The network propagates state, but local correctness does not depend on it |
| Observable runtime        | CLI commands expose status, peers, sync state, store state, and metadata  |

## What Softadastra is not

Softadastra is not:

- a Dropbox clone
- a Google Drive clone
- only a database
- only a networking library
- only a CLI tool
- a replacement for application-specific business logic

Softadastra is the synchronization foundation underneath resilient applications.

## Build

Softadastra uses CMake internally. The recommended developer workflow is through Vix.

```bash
vix build --build-target all -v
```

Build the CLI application:

```bash
vix build --build-target all -v -- \
  -DSOFTADASTRA_BUILD_APPS=ON \
  -DSOFTADASTRA_BUILD_CLI_APP=ON
```

Build the CLI and node daemon:

```bash
vix build --build-target all -v -- \
  -DSOFTADASTRA_BUILD_APPS=ON \
  -DSOFTADASTRA_BUILD_CLI_APP=ON \
  -DSOFTADASTRA_BUILD_NODE_APP=ON
```

Build in release mode:

```bash
vix build --preset release
```

Build with CMake directly:

```bash
cmake -S . -B build \
  -DSOFTADASTRA_BUILD_APPS=ON \
  -DSOFTADASTRA_BUILD_CLI_APP=ON \
  -DSOFTADASTRA_BUILD_NODE_APP=ON

cmake --build build
```

Run the CLI:

```bash
./build/apps/cli/softadastra
```

Run the node daemon:

```bash
./build/apps/node/softadastra-node
```

## Installation

```bash
cmake --install build
```

After installation:

```bash
softadastra
softadastra-node
```

## Documentation

For full documentation, visit [docs.softadastra.com](https://docs.softadastra.com).

The documentation includes:

- concepts
- architecture
- CLI usage
- C++ SDK
- JavaScript SDK
- engine internals
- guides
- reference
- releases

## Design principles

- Local state is real state
- Every accepted operation must be durable
- The network is optional
- Recovery must be deterministic
- Synchronization must be observable
- Modules must stay independent
- Applications compose modules
- The CLI must stay simple and product-friendly

## Roadmap

- [x] Core primitives
- [x] Filesystem snapshot and watcher layer
- [x] WAL writer, reader, and replay
- [x] WAL-backed store engine
- [x] Sync outbox, queue, retry, and ack tracking
- [x] TCP transport layer
- [x] Event-driven async transport backend
- [x] UDP discovery layer
- [x] Metadata layer
- [x] CLI framework module
- [x] Product CLI application
- [x] Interactive CLI session
- [x] Long-running node daemon
- [ ] Persistent sync outbox
- [ ] Real multi-operation sync batching
- [ ] Stronger peer identity handshake
- [ ] Encryption layer
- [ ] Cross-platform production backends
- [ ] SDK-level public API
- [ ] Production documentation

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

Licensed under the Apache License, Version 2.0.

See the [LICENSE](LICENSE) file for details.
