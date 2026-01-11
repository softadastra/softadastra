<p align="center">
  <img 
    src="https://res.cloudinary.com/dwjbed2xb/image/upload/v1762624310/logo-softadastra_ksv72b.png" 
    alt="Softadastra Logo" 
    width="50%"
    style="max-width:900px;border-radius:8px;"
  />
</p>

<h1 align="center">Softadastra</h1>

<p align="center">
  🌍 Offline-first · Local-first · Resilient Web Runtime
</p>

<p align="center">
  <img src="https://img.shields.io/badge/status-foundation%20draft-blue" />
  <img src="https://img.shields.io/badge/focus-offline--first%20web-success" />
  <img src="https://img.shields.io/badge/cloud-optional-lightgrey" />
</p>

---

## What is Softadastra?

**Softadastra is a foundational web runtime designed to make the web work everywhere without relying on ideal conditions.**
It is **offline-first** and **local-first**, built around a robust **synchronization engine** (WAL, outbox, retries, conflict handling), **secure peer-to-peer transport**, and **edge nodes** for caching and store-and-forward.

> **The cloud is optional. It is never required for correctness.**

Softadastra is not a product it is a **foundation** on which resilient systems can be built.

---

## Why Softadastra exists

The modern web is built on fragile assumptions:

- permanent connectivity
- low latency
- centralized cloud availability

In reality, these assumptions fail not only in developing regions, but everywhere.

Softadastra starts from a different premise:

- networks are unreliable
- failures are normal
- local execution is mandatory

Applications built on Softadastra continue to function **offline**, accept **local writes**, and **converge safely** once connectivity returns.

---

## What Softadastra is and is not

### Softadastra **is**

- a **foundational runtime**, not a single app
- a system centered on **durable state & synchronization**
- designed for **resilience by construction**
- applicable to _all_ environments, not a niche

### Softadastra **is not**

- a frontend framework
- a cloud-first SaaS platform
- a degraded version of the web

See explicit boundaries: [`docs/foundation/non-goals.md`](foundation/non-goals.md)

---

## Architecture at a glance

Softadastra is organized around a clear architectural center: **the sync engine**.

Four major layers work together:

1. **Runtime** : local execution environment
2. **Sync Engine** : WAL, outbox, retries, conflict handling
3. **Network** : secure P2P transport, routing, relays
4. **Edge** : caching and store-and-forward nodes

<p align="center">
  <img 
    src="https://res.cloudinary.com/dwjbed2xb/image/upload/v1767929305/roadmap_lgiljy.png"
    alt="Softadastra Architecture Overview"
    width="90%"
    style="max-width:1000px;border-radius:12px;"
  />
</p>

Start here: [`docs/architecture/overview.md`](architecture/overview.md)

---

## Ecosystem & stacks

Softadastra is **multi-stack by design**.

Current reference stacks include:

- **Vix.cpp** : high-performance C++ runtime (HTTP, WebSocket, P2P)
- **Ivi.php** : optional bridge stack for adoption and integration

Other stacks may exist, as long as they respect the same primitives.

See: [`docs/ecosystem/stacks.md`](ecosystem/stacks.md)

---

## Products

Products in the Softadastra ecosystem **prove** the foundation they do not define it.

Example:

- **Softadastra Drive** : local-first storage with resilient sync

Products are intentionally constrained and replaceable.

See: [`docs/ecosystem/products.md`](ecosystem/products.md)

## Build

Softadastra is an umbrella workspace that mounts its modules as Git submodules.
You clone once, initialize submodules, then build only the modules you need.

### Requirements

- Git
- CMake 3.20 or newer
- A C++20 compiler (GCC, Clang, or MSVC)

### Quick start

```bash
git clone https://github.com/softadastra/softadastra.git
cd softadastra
git submodule update --init --recursive

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Configure modules

Modules are enabled explicitly via CMake options.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DSA_ENABLE_SYNC=ON \
  -DSA_ENABLE_NET=ON \
  -DSA_ENABLE_EDGE=OFF \
  -DSA_ENABLE_ADAPTER_DESKTOP=OFF \
  -DSA_ENABLE_ADAPTER_WEB=OFF \
  -DSA_ENABLE_ADAPTER_BACKEND=OFF
```

### Build examples and tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DSA_BUILD_EXAMPLES=ON \
  -DSA_BUILD_TESTS=ON

cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Common issues

Missing submodules:

```bash
git submodule update --init --recursive
```

### Clean rebuild:

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Governance & roadmap

Softadastra is a **long-term foundation effort**.

- Roadmap: [`docs/governance/roadmap.md`](governance/roadmap.md)
- Contributions: [`docs/governance/contribution.md`](governance/contribution.md)

Architectural correctness comes before growth.

---

## Community

Softadastra values depth, clarity, and technical rigor.

- Communication: [`docs/community/communication.md`](community/communication.md)
- Events: [`docs/community/events.md`](community/events.md)

---

## License

This repository is licensed under the Apache License 2.0.

```

```
