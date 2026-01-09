# Softadastra Products in the Ecosystem

Softadastra is a foundational architecture. Products built under the Softadastra name exist to **demonstrate, validate, and stress-test** the foundation — not to define it.

This document explains the role of products within the Softadastra ecosystem.

---

## Products are expressions, not the foundation

In Softadastra:

- the foundation defines principles and primitives
- products apply those primitives to real-world problems

The direction of dependency is strict:

> **Products depend on the foundation. The foundation never depends on products.**

Any product that requires bending core principles is invalid.

---

## Purpose of products

Products serve several critical roles:

- validate architectural decisions under real constraints
- expose edge cases and failure modes
- provide concrete reference implementations
- demonstrate viability to engineers and stakeholders

Products are tools for learning and validation, not shortcuts to redefine the architecture.

---

## Softadastra Drive

**Softadastra Drive** is the first reference product.

It exists to validate:

- local-first data ownership
- durable intent recording (WAL)
- resilient synchronization
- operation replay and recovery
- behavior under prolonged disconnection

Drive is intentionally limited in scope.

Its value is not feature richness, but architectural proof.

---

## Constraints on all products

All Softadastra products must:

- function offline
- accept writes locally
- survive crashes and restarts
- converge correctly after reconnection
- tolerate partial connectivity

If a product cannot satisfy these constraints, it does not belong in the Softadastra ecosystem.

---

## Products are replaceable

Products are not permanent.

They may be:

- replaced
- rewritten
- removed

The foundation must remain stable regardless of product lifecycle.

---

## What products are not

Products are not:

- the source of architectural truth
- drivers of short-term compromises
- marketing showcases

They exist to strengthen the foundation, not to overshadow it.

---

## Future products

Future products may include:

- additional local-first applications
- developer tooling
- SDKs and utilities
- reference edge deployments

Each product must clearly state which part of the foundation it exercises.

---

## Summary

Products in Softadastra exist to prove the foundation under real-world conditions.

They are intentionally constrained, replaceable, and subordinate to the architectural core.

This separation ensures that Softadastra remains coherent, resilient, and focused over time.
