# Softadastra Principles

These principles define the non-negotiable rules that guide all technical, organizational, and product decisions within Softadastra.

They are intentionally strict. Any decision that violates these principles is considered incorrect, even if it appears beneficial in the short term.

---

## 1. Offline-first is mandatory

All systems must remain functional without network connectivity.

- Offline is not an error state
- Features must degrade gracefully, not fail
- No critical path may require live network access

If a feature cannot work offline, it does not belong in the foundation.

---

## 2. Local-first execution

All writes must be accepted locally before any synchronization occurs.

- Local state is authoritative
- The network is used to synchronize, not to operate
- User actions must never block on remote services

Remote acknowledgment is an optimization, not a requirement.

---

## 3. Durability before availability

Softadastra prioritizes durability of intent over immediate global consistency.

- Every meaningful action must be recorded durably (WAL)
- Data loss is unacceptable
- Temporary inconsistency is acceptable

Systems must recover state after crashes, restarts, or long disconnections.

---

## 4. Synchronization is a core primitive

Synchronization is not a background task or a feature — it is a first-class primitive.

- WAL, outbox, retries, and conflict handling are core components
- APIs must be designed around state transitions, not requests
- Stateless assumptions are explicitly rejected

---

## 5. Failure is the default case

All components must assume failure by default.

- Networks fail
- Nodes disappear
- Messages arrive late or duplicated

Designs that assume ideal behavior are invalid.

---

## 6. Cloud is optional, never required

Softadastra must remain functional without any centralized cloud service.

- Cloud services may improve performance or reach
- Cloud services must not be single points of failure
- Cloud dependency must always be removable

---

## 7. Minimal dependencies

Dependencies are treated as long-term liabilities.

- Prefer self-contained implementations
- Avoid deep dependency trees
- External libraries must justify their long-term cost

Replacing a dependency must always be feasible.

---

## 8. Performance is a property, not a feature

Performance must emerge from architecture, not from late optimization.

- Predictable behavior is preferred over peak benchmarks
- Local operations must be fast by default
- Slow paths must be explicit and observable

---

## 9. Modularity without fragmentation

Softadastra is modular by design, but modules must compose coherently.

- Clear boundaries
- Explicit contracts
- No hidden coupling

Modularity that increases fragmentation is considered a failure.

---

## 10. Foundation before products

The foundation always takes precedence over any product built on top of it.

- Products must adapt to the foundation
- The foundation must not bend to short-term product pressure
- Long-term correctness outweighs short-term adoption

---

## 11. Clarity over cleverness

Designs must be understandable by senior engineers reading them months later.

- Explicit over implicit
- Simple over clever
- Documented over assumed

---

## Summary

These principles are not guidelines — they are constraints.

They exist to protect Softadastra from erosion over time and to ensure that the foundation remains coherent, resilient, and future-proof.

Any contribution or decision must be evaluated against these principles before acceptance.
