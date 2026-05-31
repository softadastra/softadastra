<table>
  <tr>
    <td valign="top" width="70%">

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

<h3>Offline-first sync and storage for reliable applications.</h3>

<p>
  Softadastra is a local-first synchronization foundation built to keep data
  durable, recoverable, and eventually synchronized under unstable networks,
  offline periods, crashes, and real-world failure conditions.
</p>

<p>
  <a href="https://softadastra.com"><b>Website</b></a> ·
  <a href="https://docs.softadastra.com"><b>Docs</b></a> ·
  <a href="https://github.com/softadastra/softadastra"><b>Source</b></a>
</p>

</td>

<td valign="middle" width="15%" align="right">

<img
src="https://res.cloudinary.com/dwjbed2xb/image/upload/v1767987144/android-chrome-512x512_yjmz55.png"
width="150"
style="border-radius:50%; object-fit:cover;"
/>

</td>
  </tr>
</table>

Softadastra helps applications work when the network is not reliable.

It gives you a foundation for local writes, durable storage, sync queues, retries, peer transport, discovery, metadata, and CLI-based observability.

> Write locally. Persist first. Sync later.

<p align="center">
  <img
    src="https://res.cloudinary.com/dwjbed2xb/image/upload/v1778700690/architecture_ukigg6.png"
    alt="Softadastra architecture"
    width="760"
  />
</p>

## Install

Linux and macOS:

```bash
curl -fsSL https://softadastra.com/install.sh | bash
```

Windows PowerShell:

```powershell
irm https://softadastra.com/install.ps1 | iex
```

More installation options:

https://softadastra.com/install

## Quick start

Check the installed CLI:

```bash
softadastra version
```

Show runtime status:

```bash
softadastra status
```

Write local data:

```bash
softadastra store put name gaspard
```

Read it back:

```bash
softadastra store get name
```

Inspect sync state:

```bash
softadastra sync status
```

Start interactive mode:

```bash
softadastra
```

Example session:

```text
softadastra> help
softadastra> status
softadastra> node info
softadastra> store put name gaspard
softadastra> store get name
softadastra> sync status
softadastra> peers
softadastra> exit
```

## Why Softadastra ?

Most software assumes:

- the server is reachable
- the network is stable
- the cloud is the source of truth
- local state is temporary

Softadastra is built for the opposite reality.

It is designed for applications that must continue working when users go offline, networks fail, peers disappear, or synchronization must happen later.

## What Softadastra provides

- local-first writes
- WAL-backed durability
- recoverable local storage
- retryable synchronization
- peer transport
- local peer discovery
- node metadata
- observable runtime state
- product-level CLI
- reusable C++ modules

Softadastra is not just a library. It is a runtime foundation for reliable offline-first products.

## Runtime model

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
Ack, retry, converge
```

The network is used for propagation, not for local correctness.

## Core modules

```text
core        foundational primitives
fs          filesystem observation
wal         write-ahead log and replay
store       WAL-backed local state
sync        outbox, queue, retry, ack, conflict handling
transport   peer message delivery
discovery   local peer discovery
metadata    node identity and capabilities
cli         reusable command-line framework
```

The main user entry point is the `softadastra` CLI.
The modules power the runtime internally and can also be reused by higher-level products.

## Applications

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

### `softadastra-node`

The long-running node daemon.

It drives background services such as discovery polling, transport polling, sync scheduling, retry processing, and peer communication.

## Build from source

Softadastra uses CMake internally. The recommended developer workflow is through Vix.

```bash
vix build --build-target all -v
```

Build the CLI and node daemon:

```bash
vix build --build-target all -v -- \
  -DSOFTADASTRA_BUILD_APPS=ON \
  -DSOFTADASTRA_BUILD_CLI_APP=ON \
  -DSOFTADASTRA_BUILD_NODE_APP=ON
```

Run the CLI:

```bash
./build/apps/cli/softadastra
```

Run the node daemon:

```bash
./build/apps/node/softadastra-node
```

## Documentation

Read the full documentation:

https://docs.softadastra.com

The docs include concepts, architecture, CLI usage, SDKs, engine internals, guides, reference, and releases.

## What Softadastra is not

Softadastra is not a Dropbox clone, a Google Drive clone, or only a database.
It is the synchronization foundation underneath reliable local-first applications.

## Philosophy

> Softadastra is the Sync OS of the real internet.

The internet is not always stable.
The server is not always reachable.
The cloud should not be the only source of truth.

Softadastra treats local state as real state and uses the network only when it is available.

## License

Licensed under the Apache License, Version 2.0.
See the [LICENSE](LICENSE) file for details.
