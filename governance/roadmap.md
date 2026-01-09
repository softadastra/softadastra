# Softadastra Roadmap

This roadmap describes the long-term trajectory of Softadastra as a foundational web architecture.

It is **not** a feature roadmap. It defines phases of maturity, focus areas, and success criteria over time.

---

## Guiding principle

> **Softadastra optimizes for long-term architectural correctness, not short-term adoption.**

Progress is measured by resilience, coherence, and correctness — not by user counts or marketing milestones.

---

## Phase 0 — Concept validation (completed)

**Goal:** Establish a coherent foundation.

Focus:

- articulate the vision and principles
- define non-goals
- design the core architecture

Success criteria:

- clear architectural center (sync engine)
- documented assumptions and constraints
- internal consistency across documents

---

## Phase 1 — Foundation build (0–18 months)

**Goal:** Prove that the architecture works under real conditions.

Focus areas:

- runtime implementations
- sync engine robustness (WAL, outbox, retries)
- secure P2P transport
- edge and store-and-forward patterns

Activities:

- develop reference implementations
- build minimal reference products (e.g. Softadastra Drive)
- test prolonged offline scenarios
- validate crash recovery and convergence

Success criteria:

- no data loss under expected failures
- predictable recovery behavior
- measurable convergence under intermittent connectivity

---

## Phase 2 — Targeted adoption (18–48 months)

**Goal:** Enable external teams to adopt parts of the foundation.

Focus areas:

- documentation and clarity
- stable contracts between layers
- SDKs and integration tooling
- education and reference material

Activities:

- publish architectural guides
- support partial adoption (runtime only, sync only, etc.)
- collaborate with early adopters

Success criteria:

- independent teams using Softadastra primitives
- successful deployments in constrained environments
- minimal need for architectural changes

---

## Phase 3 — Ecosystem consolidation (4–7 years)

**Goal:** Establish Softadastra as a credible reference architecture.

Focus areas:

- governance processes
- compatibility and versioning
- long-term maintenance

Activities:

- formalize contribution models
- define compatibility policies
- maintain reference implementations

Success criteria:

- stable architectural contracts
- growing but controlled contributor base
- consistent behavior across implementations

---

## Phase 4 — Implicit standard (7–10 years)

**Goal:** Become an obvious choice for resilient web systems.

Focus areas:

- standardization
- interoperability
- long-term sustainability

Activities:

- participate in standards discussions
- publish formal specifications
- support long-lived deployments

Success criteria:

- Softadastra principles reflected in industry designs
- third-party implementations
- architectural longevity

---

## What this roadmap avoids

This roadmap deliberately avoids:

- aggressive growth targets
- market-share milestones
- feature checklists

Those are consequences of correctness, not drivers of it.

---

## Summary

Softadastra is a long-term foundation effort.

This roadmap prioritizes depth, resilience, and coherence over speed, ensuring that the architecture remains viable and relevant for decades.
