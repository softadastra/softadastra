# Softadastra Sync Engine Architecture

This document describes the **synchronization engine**, the core primitive of Softadastra.

Softadastra is not built around request–response APIs. It is built around **durable state transitions** that can be replayed, synchronized, and reconciled under unreliable conditions.

---

## Why sync is the center

In traditional systems:

- the server is authoritative
- the client depends on immediate network success

In Softadastra:

- the local device is authoritative for user intent
- the network is unreliable by default
- synchronization is the mechanism that creates convergence over time

The sync engine exists to ensure:

- no loss of intent
- recoverability after failure
- eventual convergence of state

---

## Core concepts

### State transition

A **state transition** is a meaningful change to application state (create, update, delete, rename, etc.).

Softadastra treats state transitions as first-class objects.

### Operation

An **operation** is the portable representation of a state transition.

Operations must be:

- deterministic
- serializable
- replayable

### Convergence

**Convergence** is the process by which multiple replicas move toward compatible state over time.

Softadastra favors convergence under failure rather than strict immediate consistency.

---

## Components

### 1. Write-Ahead Log (WAL)

The WAL is an append-only log of operations.

Responsibilities:

- durably record every state transition
- preserve order (per-writer) for replay
- allow recovery after crash

Requirements:

- append-only (immutability)
- fsync / durability guarantees (per policy)
- readable and replayable

The WAL is the source of truth for local intent.

---

### 2. Outbox

The outbox is a queue of operations that must be delivered to other replicas.

Responsibilities:

- track pending deliveries
- support retries and backoff
- batch operations
- maintain delivery metadata (attempts, last error, last attempt time)

The outbox decouples local writes from network success.

---

### 3. Retry & backoff policy

Network failures are expected.

Retry behavior must:

- be exponential by default
- include jitter
- respect network conditions
- avoid thundering herds

The retry policy is a core part of correctness, not a performance detail.

---

### 4. Acknowledgments and progress tracking

Synchronization requires tracking what has been received.

Mechanisms may include:

- per-peer sequence acknowledgments
- watermarks
- per-batch acknowledgments

Progress tracking must be:

- durable
- resumable after restarts
- resistant to duplicates

---

### 5. Snapshotting

Replaying an unbounded WAL is inefficient.

Snapshots provide:

- a compact representation of current state
- faster startup and recovery
- bounded replay cost

Snapshots must be:

- derivable from WAL
- consistent with recorded operations

---

## Conflict handling

Conflicts occur when replicas perform incompatible operations.

Softadastra does not mandate a single conflict strategy.

However, conflict handling must follow these rules:

- conflicts must be detectable
- resolution must be explicit
- no silent data loss

Possible strategies:

- last-writer-wins (with clear semantics)
- version vectors / causal tracking (future)
- preserving both versions (conflict copies)

The foundation prioritizes safety and explicitness.

---

## Idempotency and deduplication

Messages can be duplicated.

Therefore:

- operations must be idempotent or safely deduplicated
- delivery must tolerate replays
- receivers must treat duplicates as normal

Idempotency is required for correctness under unreliable transport.

---

## Ordering and causality

Softadastra does not assume global ordering.

It may rely on:

- per-writer ordering
- causal metadata (when required)

The sync engine must remain correct when messages arrive:

- late
- out of order
- duplicated

---

## Observability

Sync correctness must be observable.

The engine should expose:

- queue depth
- retry rates
- ack progress
- conflict counts
- time-to-converge metrics

Observability is part of reliability.

---

## Summary

The Softadastra sync engine is the foundation’s core.

It guarantees durable recording of intent (WAL), decouples local writes from delivery (outbox), and enables convergence through retries, acknowledgments, snapshots, and explicit conflict handling.

This design ensures that applications continue to function and recover correctly under unreliable and intermittent conditions.
