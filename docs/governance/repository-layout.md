# Repository Layout, Submodules, and Versioning Rules

This document defines the mandatory rules governing the structure, layout, and versioning of repositories in the Softadastra ecosystem.

These rules are non-negotiable.
Any implementation or contribution that violates them must be rejected.

---

## 1. Purpose of this document

This document exists to:

- preserve architectural correctness over time
- prevent accidental coupling between modules
- guarantee long-term maintainability
- allow independent evolution of modules
- ensure predictable adoption for organizations

Convenience must never override correctness.

---

## 2. Repository model

Softadastra follows a central umbrella repository model combined with independent module repositories.

### Canonical repository

softadastra/softadastra

This repository is the architectural authority.

It defines:

- principles and non-goals
- architectural invariants
- module contracts
- governance rules
- compatibility requirements

It may include a unified build workspace.

It does not contain core business logic.

---

## 3. Module repositories

Each functional component of Softadastra is implemented in its own repository.

Examples:

- softadastra-core
- softadastra-sync
- softadastra-net
- softadastra-edge
- softadastra-adapter-\*
- softadastra-tools
- softadastra-drive

Module repositories must:

- be buildable independently
- export a clear public interface
- declare their dependencies explicitly
- remain replaceable

---

## 4. Submodule integration rules

The umbrella repository integrates module repositories using Git submodules.

### Mandatory rules

1. All modules must be mounted under the modules/ directory, except tooling.
2. Submodules must point to a specific commit or tagged release.
3. Floating branches are forbidden.
4. Recursive submodules are allowed only if explicitly documented.
5. The umbrella repository must remain buildable after submodule initialization.

### Initialization

Users must be able to initialize all modules using:

git submodule update --init --recursive

Failure to do so must result in a clear error message during configuration.

---

## 5. Module layout requirements

Each module repository must follow these minimal layout rules:

include/softadastra/<module>/
src/
tests/
CMakeLists.txt
README.md
LICENSE

Optional directories:

- examples/
- tools/
- docs/

The public API must live exclusively under include/softadastra/<module>/.

---

## 6. Build and target conventions

Each module must export exactly one primary CMake target.

### Naming

- Target name: softadastra::<module>
- Example: softadastra::sync

Alias targets are allowed but must resolve to the canonical name.

### Visibility

- Public headers must be explicitly marked as PUBLIC.
- Internal headers must not be installed or exported.
- Compile options must not leak unintentionally.

---

## 7. Dependency rules

Dependencies must respect the architectural layering.

Mandatory constraints:

- softadastra-core must have no dependencies on other Softadastra modules.
- softadastra-sync may depend only on softadastra-core.
- softadastra-net may depend only on softadastra-core.
- softadastra-edge may depend on core, net, and optionally sync.
- adapters may depend on core, sync, and net.
- products may depend on all modules.

Circular dependencies are forbidden.

Violations must be rejected at review time.

---

## 8. Versioning rules

Softadastra uses semantic versioning with strict interpretation.

### Module versioning

Each module repository:

- defines its own version
- increments versions independently
- documents breaking changes clearly

Version format:

MAJOR.MINOR.PATCH

Rules:

- MAJOR increments indicate breaking API or semantic changes
- MINOR increments indicate backward-compatible feature additions
- PATCH increments indicate bug fixes only

---

## 9. Umbrella versioning

The umbrella repository version represents:

- a tested combination of module versions
- a stable integration snapshot
- a reference point for adoption

The umbrella version does not override module versions.

Upgrading the umbrella version may:

- advance submodule pointers
- enable or disable modules
- update build or governance rules

---

## 10. Compatibility guarantees

A module may declare compatibility with:

- a range of umbrella versions
- specific protocol versions

Compatibility must be explicit.

Silent breakage is unacceptable.

---

## 11. Backward compatibility policy

Softadastra does not guarantee backward compatibility with:

- legacy web assumptions
- cloud-only architectures
- stateless-only models

However:

- internal invariants must remain stable
- protocol compatibility must be versioned
- migrations must be documented

---

## 12. Contribution and review enforcement

All contributions must be reviewed against:

- architectural principles
- repository layout rules
- dependency constraints
- versioning policy

Any change that weakens boundaries or introduces hidden coupling must be rejected.

---

## Summary

These rules exist to protect Softadastra from erosion over time.

They prioritize:

- correctness over convenience
- clarity over cleverness
- long-term stability over short-term adoption

Violations of these rules are architectural failures, not stylistic differences.
