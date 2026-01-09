# Softadastra Network Architecture (P2P, Edge, Relay)

This document describes Softadastra’s networking layer.

Softadastra does not assume stable connectivity or centralized servers. The network layer is designed to move synchronization data across unreliable environments using **secure peer-to-peer transport**, **edge relays**, and **store-and-forward** patterns.

---

## Role of the network layer

The network layer exists to transport synchronization data.

It does **not** define application semantics.

Responsibilities:

- discover reachable peers and routes
- establish secure channels
- deliver operations and acknowledgments
- tolerate partial connectivity
- support relays and store-and-forward paths

---

## Core design principles

### Unreliable-by-default

The network must assume:

- intermittent connectivity
- NATs and firewalls
- asymmetric reachability
- packet loss and jitter

Designs that require stable end-to-end connectivity are invalid.

### Secure-by-default

All peer links must be secured.

The network layer must provide:

- authenticated peers
- encrypted payloads
- integrity protection

### Transport independence

The sync engine must not be coupled to a specific transport.

The network layer provides transport adapters (direct, relay, edge).

---

## Communication model

Softadastra exchanges:

- operations (WAL records)
- acknowledgments (progress)
- control messages (handshake, ping/pong)

The system favors small, explicit messages that can be retried and deduplicated.

---

## Peer lifecycle

### 1. Discovery

Peers can be discovered via multiple mechanisms, such as:

- static bootstrap addresses
- local network discovery
- directory services (optional)

Discovery is modular and environment-specific.

### 2. Handshake

A handshake establishes:

- protocol compatibility
- peer identity
- session parameters
- session keys for encryption

Handshake must be designed for compatibility and upgradeability.

### 3. Secure channel

After handshake:

- messages are encrypted and authenticated (AEAD)
- session keys are used
- nonces are managed per peer

### 4. Keepalive and liveness

The network layer monitors peer liveness:

- ping/pong
- timeouts
- disconnect detection

Liveness is required for outbox scheduling and retry decisions.

---

## Routing and relaying

Direct peer-to-peer connectivity is not always possible.

Therefore, Softadastra supports:

### Relay routing

- a peer can forward messages between two peers
- relays are opportunistic and replaceable

### Store-and-forward

- if the destination is offline
- a relay or edge node can temporarily store messages
- delivery resumes when reachable

Store-and-forward is central for environments with intermittent reachability.

---

## Edge nodes

Edge nodes are network participants optimized for:

- proximity
- availability
- buffering

They may provide:

- message relay
- store-and-forward queues
- caching of frequently accessed data
- optional indexing services

Edge nodes improve performance and reach without creating a mandatory central authority.

---

## Message properties

### Idempotency and dedup

Receivers must tolerate:

- duplicated messages
- replays
- out-of-order arrival

Messages must carry enough metadata to support:

- deduplication
- ack tracking
- safe retries

### Backpressure

To prevent overload:

- the network layer must support backpressure
- send queues must be bounded
- retry policies must respect congestion

---

## Vix.cpp as a reference transport stack

Vix.cpp provides a reference implementation of the Softadastra network layer.

It includes:

- high-performance HTTP and WebSocket transport
- secure P2P channels
- peer discovery and messaging primitives

The Softadastra network layer may use other stacks, but they must satisfy the same guarantees.

---

## Summary

Softadastra networking is designed to reliably move synchronization data under real-world constraints.

It is secure-by-default, tolerant to intermittent connectivity, and supports relays and store-and-forward through peers and edge nodes.

This enables Softadastra systems to converge even when direct connectivity is unavailable.
