# Versioning Policy

This document defines the official versioning rules for the Softadastra ecosystem.

Versioning is not cosmetic.
It is a core part of the architectural contract.

---

## 1. Goals of versioning

The Softadastra versioning policy exists to:

- guarantee long-term stability
- make upgrades predictable
- prevent silent breakage
- allow independent evolution of modules
- support enterprise-grade adoption

If versioning is unclear, the system is broken.

---

## 2. Semantic Versioning (strict)

All Softadastra repositories use **Semantic Versioning**:

MAJOR.MINOR.PATCH

This rule applies to:

- module repositories
- products
- umbrella releases

There are no exceptions.

---

## 3. Meaning of versions

### PATCH

Incremented when:

- fixing bugs
- improving performance
- refactoring internals without changing behavior

PATCH releases must be safe to upgrade without code changes.

---

### MINOR

Incremented when:

- adding new features
- extending APIs in a backward-compatible way
- adding optional capabilities

MINOR releases must not break existing integrations.

---

### MAJOR

Incremented when:

- breaking public APIs
- changing semantic behavior
- removing features
- modifying protocol guarantees

MAJOR upgrades require explicit migration.

---

## 4. Module versioning

Each Softadastra module:

- owns its own version
- releases independently
- documents breaking changes clearly

Examples:

- softadastra-core v1.4.2
- softadastra-sync v0.9.0
- softadastra-net v1.1.0

Modules must never assume synchronized version numbers.

---

## 5. Umbrella versioning

The umbrella repository (softadastra):

- represents a **validated combination** of module versions
- pins submodules to exact commits or tags
- does not override module versions

The umbrella version answers:
"Which versions are known to work together?"

---

## 6. Compatibility declarations

Modules must declare:

- supported umbrella versions
- supported protocol versions (if applicable)

Compatibility must be explicit and documented.

Implicit compatibility is forbidden.

---

## 7. Protocol versioning

Network and sync protocols must be versioned independently.

Rules:

- protocol versions are immutable once released
- backward compatibility must be explicit
- incompatible changes require a new protocol version

Negotiation must happen at runtime.

---

## 8. Pre-1.0 policy

Versions below 1.0 indicate:

- active design iteration
- unstable public APIs
- experimental semantics

However:

- breaking changes must still bump MINOR
- behavior must never change silently

---

## 9. Deprecation policy

Deprecation must follow this process:

1. Mark as deprecated in documentation
2. Emit warnings (where applicable)
3. Keep support for at least one MINOR cycle
4. Remove only in the next MAJOR

Instant removal is forbidden.

---

## 10. Version pinning

Consumers are strongly encouraged to:

- pin exact versions in production
- upgrade deliberately
- test upgrades in isolation

Floating dependencies are discouraged.

---

## 11. What versioning does NOT guarantee

Versioning does not guarantee:

- backward compatibility with broken assumptions
- compatibility with cloud-only systems
- preservation of incorrect usage patterns

Correctness comes first.

---

## Summary

Versioning in Softadastra is a contract.

It exists to protect:

- users
- integrators
- the architecture itself

Breaking this contract is an architectural failure.
