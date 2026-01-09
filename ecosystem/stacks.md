# Softadastra Ecosystem Stacks

Softadastra is a foundational architecture, not a single implementation.

This document explains how multiple technology stacks can coexist within the Softadastra ecosystem while remaining aligned with the same core principles.

---

## Foundation vs. implementations

Softadastra defines:

- architectural principles (offline-first, local-first, durability)
- core primitives (WAL, outbox, retries, conflict handling)
- network model (secure P2P, relay, edge)

Softadastra does **not** mandate:

- a single programming language
- a single runtime
- a single deployment model

Any stack is acceptable if it conforms to the foundation principles.

---

## Primary reference stack: Vix.cpp

**Vix.cpp** is the primary reference implementation for Softadastra’s runtime and networking layers.

It is designed for:

- high-performance HTTP serving
- WebSocket real-time channels
- secure peer-to-peer transport
- predictable threading and memory behavior

In Softadastra, Vix.cpp commonly implements:

- Runtime APIs (HTTP/WS)
- P2P transport and routing
- Edge node services
- Sync engine components (where applicable)

Vix.cpp is a strong fit when performance, control, and reliability are primary constraints.

---

## Bridge stack: Ivi.php

**Ivi.php** exists as an optional bridge stack for adoption and integration.

Its role is not to replace the foundation, but to:

- help teams adopt Softadastra principles incrementally
- integrate with existing PHP ecosystems
- provide compatibility layers where needed

Ivi.php may be used for:

- user-facing web applications
- business logic integration
- legacy system interoperability

It must still respect Softadastra’s architectural constraints:

- offline-first and local-first behaviors
- durable intent recording
- explicit sync boundaries

---

## Multiple stacks, one architecture

Softadastra encourages an ecosystem where:

- multiple stacks can implement the same primitives
- modules can be adopted independently
- systems can evolve without lock-in

This is critical for long-term adoption.

---

## Interoperability requirements

To ensure coherence, all stacks must share:

### 1. A stable operation model

- a clear operation schema
- deterministic semantics
- versioned compatibility rules

### 2. Transport compatibility (when required)

- message envelopes
- acknowledgments
- encryption expectations

### 3. Clear boundaries

- runtime vs sync engine vs transport
- no hidden coupling

Interoperability is achieved through explicit contracts, not implicit conventions.

---

## Future stacks

Softadastra may later include additional stacks such as:

- Python-based tooling
- embedded runtimes
- mobile-specific runtimes
- specialized edge deployments

The foundation remains unchanged.

---

## Summary

Softadastra is a multi-stack ecosystem grounded in a single architectural foundation.

Vix.cpp acts as the performance-focused reference stack.

Ivi.php acts as an adoption bridge and integration layer.

Other stacks may be added over time, as long as they respect the same primitives, boundaries, and principles.
