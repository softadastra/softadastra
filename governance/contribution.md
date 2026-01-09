# Softadastra Contribution Model

This document defines how contributions to Softadastra are handled.

Because Softadastra is a foundational architecture, contribution rules are intentionally strict. The goal is to preserve coherence, correctness, and long-term viability.

---

## Contribution philosophy

> **Softadastra values correctness and coherence over velocity.**

Not all contributions are equal, and not all parts of the system are equally open to change.

The foundation must remain stable and internally consistent.

---

## What can be contributed

Contributions are welcome in the following areas:

### 1. Documentation

- clarifications
- corrections
- architectural explanations
- examples and diagrams

Documentation contributions are encouraged and prioritized.

---

### 2. Reference implementations

Depending on the repository:

- runtime components
- sync engine components
- network and transport layers

Only contributions that respect the documented principles and architecture will be accepted.

---

### 3. Tooling and utilities

Contributions may include:

- developer tools
- testing utilities
- observability tooling

These must not introduce unnecessary dependencies or complexity.

---

## What is not open for contribution

The following are not open for external contribution:

- core architectural principles
- foundational assumptions
- non-goals
- long-term roadmap direction

These are defined and maintained by the core maintainers.

---

## Contribution process

1. Open an issue describing the proposal
2. Clearly state the motivation and impact
3. Reference relevant principles and documents
4. Discuss trade-offs explicitly

Pull requests without prior discussion may be closed.

---

## Review criteria

All contributions are evaluated against:

- alignment with Softadastra principles
- impact on resilience and correctness
- long-term maintenance cost
- clarity and explicitness

Short-term gains do not justify long-term architectural risk.

---

## Maintainers and governance

Softadastra is governed by a small group of core maintainers.

Responsibilities:

- protect the architectural integrity
- approve or reject changes
- guide long-term evolution

Decisions prioritize the foundation over individual preferences.

---

## Code of conduct

All contributors are expected to engage respectfully and constructively.

Technical disagreement is welcome. Personal attacks are not.

---

## Summary

Softadastra welcomes thoughtful, well-motivated contributions.

However, preserving the architectural foundation is the highest priority.

If a contribution compromises coherence or resilience, it will not be accepted.
