# Foundation Invariants

This document defines the non-negotiable invariants of the Softadastra foundation.

These invariants are stronger than guidelines.
They are architectural laws.

Any implementation, module, product, or contribution that violates these invariants must be rejected.

---

## 1. Offline-first is mandatory

The system must remain functional without network connectivity.

- Offline is a normal operating mode, not an error state.
- Critical paths must never require live network access.
- Features must degrade gracefully, not fail catastrophically.
- If something cannot work offline, it does not belong in the foundation.

---

## 2. Local writes are authoritative

All writes must be accepted locally first.

- The local device is authoritative for user intent.
- The network exists for synchronization, not for primary operation.
- Remote acknowledgement is an optimization, not a requirement.
- The user must never be blocked on remote services.

---

## 3. Durable intent is required

Every significant state transition must be recorded durably.

- Operations must be appended to a durable log (WAL or equivalent).
- Data loss is unacceptable in normal operation.
- Temporary inconsistency is acceptable.
- Recovery after crash, reboot, or long disconnection must be possible.

---

## 4. Synchronization is a primitive

Synchronization is not a background task or feature.

Softadastra systems must treat synchronization as a first-class mechanism.

Required components include:

- WAL (durable intent)
- outbox (delivery tracking)
- retries and backoff
- acknowledgement and progress tracking
- conflict detection and explicit resolution hooks
- snapshots or state reconstruction mechanisms

Stateless request-response assumptions are explicitly rejected.

---

## 5. Failure is the default case

Systems must be correct under failure.

Assume:

- networks fail
- nodes disappear
- messages arrive late, duplicated, or out of order
- connectivity can be partial and asymmetric

Designs that assume ideal behavior are invalid.

---

## 6. Cloud is optional

Softadastra must remain correct without any cloud service.

- Cloud can improve performance, reach, or durability.
- Cloud services must not be single points of failure.
- The architecture must allow cloud dependencies to be removed.
- Correctness must never depend on centralized infrastructure.

---

## 7. Explicit boundaries are required

Modules must have clear boundaries.

- contracts must be explicit
- coupling must be visible
- hidden cross-module dependencies are forbidden
- fragmentation is failure

Modularity is required, but it must remain coherent.

---

## 8. Determinism is required for core operations

Core operations must be deterministic.

- identical inputs must produce identical state transitions
- side effects must be explicit and observable
- non-determinism must be isolated

Determinism is required for replay, recovery, and conflict handling.

---

## 9. Idempotence or safe deduplication is required

Transport is not reliable.

Therefore:

- operations must be idempotent, or
- the system must safely deduplicate operations

Receivers must tolerate:

- duplication
- replay
- partial delivery
- out-of-order arrival

---

## 10. Observability is part of correctness

Correctness must be observable.

The foundation must expose, at minimum:

- WAL progress and health
- outbox depth and backlog
- retry rate and last error
- acknowledgement progress per peer
- snapshot and rebuild status
- conflict counters and resolution outcomes
- time-to-convergence metrics (where applicable)

If correctness cannot be observed, it cannot be trusted.

---

## 11. Performance is an architectural property

Performance must follow from the design.

- local operations must be fast by default
- slow paths must be explicit
- resource usage must be predictable
- performance must degrade gracefully under load or failure

Peak benchmarks are less important than predictable behavior.

---

## 12. Minimal dependencies are mandatory

Dependencies are long-term liabilities.

- prefer self-contained implementations
- avoid deep dependency trees
- external libraries must justify long-term costs
- replacing a dependency must remain possible

---

## 13. Products must not distort the foundation

Products exist to validate the foundation.

- products depend on the foundation
- the foundation must never bend for short-term product needs
- adoption speed must not override correctness

---

## Summary

Softadastra is defined by invariants, not features.

These invariants protect the foundation from erosion and ensure that Softadastra systems remain:

- correct under failure
- usable offline
- recoverable after crashes
- convergent over time
- independent of cloud assumptions
