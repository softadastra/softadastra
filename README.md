# Softadastra

**Softadastra is a foundational web runtime that reconstructs the web to work everywhere without relying on ideal conditions.**  
It is **offline-first** and **local-first**, built around a robust **sync engine** (WAL, outbox, retries, conflict handling), **secure P2P transport**, and **edge nodes** for caching and store-and-forward.  
**The cloud is optional, never required.**

---

## Why Softadastra exists

The modern web assumes ideal infrastructure: stable connectivity, low latency, and permanent cloud availability.  
But reality is different: networks are intermittent, expensive, or unreliable—yet applications and data still need to work.

Softadastra is designed for the real world:

- Apps remain usable **offline**
- Writes are **local-first**
- Sync is robust and resumable
- Systems degrade gracefully under poor conditions

---

## What Softadastra is (and isn’t)

### Softadastra is

- A **foundation**, not a single product
- A web runtime built for **resilience**
- A coherent stack centered on **sync + transport + edge**

### Softadastra is not

- A new frontend framework
- A cloud-first SaaS platform
- “Web but for poor networks” (it’s universal, not niche)

See: [`foundation/non-goals.md`](foundation/non-goals.md)

---

## Architecture at a glance

Softadastra is organized into four pillars:

1. **Runtime** — application runtime and APIs
2. **Sync Engine** — WAL/outbox/retries/conflicts as the core primitive
3. **Network** — secure P2P transport and routing
4. **Edge** — caching and store-and-forward nodes

Start here: [`architecture/overview.md`](architecture/overview.md)

---

## Ecosystem & stacks

Softadastra can be implemented with multiple stacks. Today:

- **Vix.cpp**: high-performance C++ runtime stack (HTTP / WebSocket / P2P)
- **Ivi.php**: optional bridge stack for adoption and integration

See: [`ecosystem/stacks.md`](ecosystem/stacks.md)

---

## Products

Softadastra also builds products that **prove** the foundation, without defining it.

Example: **Softadastra Drive** (local-first storage + resilient sync)

See: [`ecosystem/products.md`](ecosystem/products.md)

---

## Roadmap & governance

Softadastra is a long-term foundation effort.

- Roadmap: [`governance/roadmap.md`](governance/roadmap.md)
- Contributing: [`governance/contribution.md`](governance/contribution.md)

---

## Community

- Communication: [`community/communication.md`](community/communication.md)
- Events: [`community/events.md`](community/events.md)

---

## License

TBD. (Softadastra will define licensing per module and per repository.)
