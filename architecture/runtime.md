# Softadastra Runtime Architecture

This document describes the role and responsibilities of the **application runtime** within the Softadastra architecture.

The runtime is responsible for executing application logic locally and providing a stable execution environment that remains functional regardless of network conditions.

---

## Purpose of the runtime

The runtime exists to ensure that applications:

- execute locally by default
- accept user actions immediately
- remain usable offline
- expose deterministic state changes to the sync engine

In Softadastra, the runtime is not a thin client — it is the primary execution environment.

---

## Local-first execution model

All application logic runs locally within the runtime.

Key properties:

- No blocking calls on remote services
- No dependency on server-side availability
- Predictable latency for user actions

Remote services may enhance behavior, but must never be required for core functionality.

---

## State ownership

The runtime owns application state.

Responsibilities:

- Manage local databases or file systems
- Maintain in-memory state
- Apply state transitions deterministically

The runtime never delegates authoritative state ownership to the network or the cloud.

---

## Interaction with the sync engine

The runtime does not perform synchronization itself.

Instead, it:

- emits explicit state changes
- writes durable records to the WAL
- hands off operations to the outbox

The sync engine is responsible for delivery, retries, and convergence.

This separation ensures clarity of responsibility.

---

## Deterministic behavior

Runtime behavior must be deterministic.

Requirements:

- Identical inputs produce identical state transitions
- Side effects are explicit and recorded
- Non-deterministic behavior is isolated

Determinism is required for replay, recovery, and conflict resolution.

---

## Performance characteristics

Performance is a foundational requirement.

The runtime must:

- provide fast local operations
- minimize memory overhead
- expose clear performance boundaries

Latency introduced by synchronization must never affect local responsiveness.

---

## Vix.cpp as a reference runtime

Vix.cpp serves as a reference implementation of a Softadastra runtime.

It provides:

- high-performance HTTP and WebSocket servers
- secure P2P networking
- predictable memory and threading models

Other runtimes may exist, but they must conform to the same architectural principles.

---

## Language and stack independence

Softadastra does not mandate a single programming language.

Requirements for any runtime implementation:

- local-first execution
- durable state emission
- explicit sync boundaries

This allows multiple stacks (C++, PHP, others) to coexist within the ecosystem.

---

## Summary

The Softadastra runtime is the execution backbone of the system.

It prioritizes locality, determinism, and performance, while delegating synchronization and networking concerns to dedicated layers.

This design ensures that applications remain responsive, reliable, and recoverable under all conditions.
