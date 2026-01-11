# Softadastra Repositories and Module Contract

This document defines the official repository model of the Softadastra ecosystem.

It specifies:

- the role of each repository
- how modules relate to the Softadastra umbrella
- the dependency rules that must never be violated
- how organizations are expected to adopt Softadastra

This document is normative.

---

## 1. Repository model overview

Softadastra follows a **central umbrella + external modules** model.

- The repository `softadastra` is the architectural center.
- All implementation modules live in their own repositories.
- The umbrella repository mounts modules using Git submodules.
- Users may clone one repository and build everything, or consume individual modules independently.

This model provides:

- architectural clarity
- strong module boundaries
- simple adoption for organizations
- long-term maintainability

Softadastra is **not monolithic**, but it is **cohesive**.

---

## 2. The umbrella repository

### Repository

softadastra/softadastra

### Role

The umbrella repository defines the **architecture**, not the implementation.

It contains:

- principles and non-goals
- architectural documentation
- governance and roadmap
- ecosystem and adoption guides
- a unified build workspace

It does not contain business logic.

The umbrella repository is the **source of truth** for all Softadastra-compatible implementations.

---

## 3. Core implementation modules

Each module is implemented in its own repository and mounted into the umbrella workspace.

### softadastra-core

**Purpose**  
Shared foundational types and invariants.

**Responsibilities**

- stable primitive types
- identifiers, versions, capabilities
- error types and result semantics
- clocks and timestamps
- minimal shared utilities

**Constraints**

- no business logic
- no network dependencies
- no storage backends
- extremely stable API

---

### softadastra-sync

**Purpose**  
Durable synchronization engine.

**Responsibilities**

- write-ahead log (WAL)
- outbox and delivery tracking
- retry and backoff policies
- acknowledgements and progress tracking
- snapshots and state reconstruction
- explicit conflict detection and resolution hooks

**Constraints**

- must function fully offline
- no dependency on concrete networking
- no cloud assumptions
- deterministic behavior required

This module is the architectural core of Softadastra.

---

### softadastra-net

**Purpose**  
Protocol definitions and transport-agnostic networking primitives.

**Responsibilities**

- message envelopes and serialization formats
- handshake and capability negotiation
- message type definitions
- protocol versioning rules
- abstract transport interfaces

**Constraints**

- no concrete networking implementation required
- no sockets, servers, or cloud services mandated
- must remain usable in constrained environments

---

### softadastra-edge

**Purpose**  
Edge and proximity infrastructure.

**Responsibilities**

- relay of synchronization messages
- store-and-forward buffering
- opportunistic caching
- proximity-based acceleration

**Constraints**

- never authoritative
- always optional
- must not be a single point of failure
- system must remain correct without it

Edge nodes improve performance and reach but are never required for correctness.

---

## 4. Adapters and integration layers

Adapters connect existing systems to Softadastra primitives.

They translate external actions into durable operations.

### softadastra-adapter-desktop

**Purpose**

- desktop and local application integration

**Responsibilities**

- filesystem and local database integration
- file watchers and batching
- mapping local actions to Softadastra operations

---

### softadastra-adapter-web

**Purpose**

- browser and web environment integration

**Responsibilities**

- IndexedDB persistence
- Service Worker integration
- WASM-based execution
- real offline-first web behavior

---

### softadastra-adapter-backend

**Purpose**

- backend and server-side system integration

**Responsibilities**

- bridging existing APIs with Softadastra sync
- exposing operation and acknowledgement endpoints
- enabling gradual adoption in legacy systems

---

## 5. Tools

### softadastra-tools

**Purpose**
Developer tooling and diagnostics.

**Responsibilities**

- CLI tools
- WAL inspection and replay
- outbox and retry diagnostics
- protocol validation
- debugging and observability helpers

Tools are optional but strongly recommended during development.

---

## 6. Products

### softadastra-drive

**Purpose**
Reference product validating the Softadastra architecture.

**Role**

- demonstrate offline-first correctness
- validate synchronization behavior
- expose real-world failure modes
- guide architectural evolution

Products do not define the architecture.
They validate it.

Products are intentionally limited and replaceable.

---

## 7. Dependency rules (non-negotiable)

The following dependency rules are mandatory:

- `softadastra-core` has no dependencies on other modules.
- `softadastra-sync` depends only on `softadastra-core`.
- `softadastra-net` depends only on `softadastra-core`.
- `softadastra-edge` may depend on `core`, `net`, and optionally `sync`.
- adapters may depend on `core`, `sync`, and `net`.
- products may depend on all modules.
- no module may require cloud services for correctness.

Circular dependencies are forbidden.

---

## 8. Adoption model for organizations

Organizations may adopt Softadastra in multiple ways:

1. Clone the umbrella repository and build selected modules.
2. Vendor a specific module such as `softadastra-sync`.
3. Replace individual modules with compatible internal implementations.
4. Deploy edge infrastructure selectively.

The only requirement is adherence to the architectural contracts defined in the umbrella repository.

Softadastra does not impose a deployment model.
It enforces architectural correctness.

---

## 9. Compatibility and evolution

Modules evolve independently but must:

- declare compatibility with the Softadastra architecture
- respect protocol and semantic versioning rules
- remain replaceable

Breaking architectural invariants is not allowed.

---

## Summary

Softadastra is a cohesive but non-monolithic ecosystem.

- The umbrella repository defines the truth.
- Modules implement replaceable primitives.
- Products validate, not dictate.
- Adoption is incremental and flexible.

This structure exists to protect the foundation over time.
