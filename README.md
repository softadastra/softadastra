# Softadastra

Softadastra is infrastructure software for turning supported machines into simple, resilient Hosts for software.

A Host provides the infrastructure around software without requiring that software to adopt a Softadastra framework, SDK, programming language, database, or application architecture.

The foundational model is simple:

```text
Machine + Softadastra = Host
```

A Host is responsible for helping software run, remain reachable, and stay available within the real capabilities of the machine and its environment.

## The Host

A Softadastra Host may be:

- an existing personal computer;
- a desktop machine;
- a mini-PC;
- a server;
- a dedicated Softadastra Box;
- another supported machine in the future.

The machine itself is not the product model.

The Host is.

```text
HOST
 |
 +-- run software
 |
 +-- make it reachable
 |
 +-- keep it available
```

Softadastra handles infrastructure around the software while the software keeps control of its own architecture.

## Why Softadastra

Running software close to its users often requires dealing with process execution, service management, networking, machine configuration, reachability, and availability.

These infrastructure concerns are necessary, but users and organizations should not need to understand every underlying mechanism simply to operate their software.

Softadastra exists to move as much of that infrastructure responsibility as possible into the Host.

The goal is not to hide physical limits or pretend every machine can run every program. The goal is to make supported infrastructure simpler, predictable, and easier to operate.

When a Host can run software, Softadastra should make that straightforward.

When it cannot, Softadastra should explain why clearly.

```text
compatible   -> run
incompatible -> explain
```

## Software independence

Softadastra does not define how hosted software must be built.

Fundamental hosting does not require:

- a Softadastra SDK;
- a Softadastra application framework;
- a specific programming language;
- a specific database;
- a specific frontend or backend architecture;
- HTTP;
- WebSocket;
- gRPC;
- a proprietary Softadastra application protocol.

Hosted software may be existing software, third-party software, customer-owned software, legacy software, or software developed specifically for an organization.

Its internal architecture belongs to its owner.

Softadastra should know only what is required at the infrastructure boundary to operate that software correctly.

The preferred direction is:

```text
adapt Softadastra to the software
```

rather than:

```text
redesign the software for Softadastra
```

## Local as a foundation

Softadastra follows a simple infrastructure principle:

> **Local is a foundation. Internet is an extension.**

When hosted software can operate locally, Softadastra should not introduce an unnecessary dependency on continuous Internet access.

```text
local users
    |
    v
   Host
    |
    v
 software
```

Internet may extend a Host with capabilities such as remote access, administration, backup, relays, or future Host-to-Host communication.

```text
local users
    |
    v
   Host
    |
    +--------- Internet --------- remote user
```

Losing Internet should not automatically make the local Host unusable when the hosted software itself does not require Internet.

Softadastra cannot remove real physical or software dependencies. If software fundamentally depends on a remote service, that dependency still exists.

## Softadastra Box

A Softadastra Box is a dedicated Host provided as a product.

```text
Dedicated machine
       +
Softadastra
       =
Host
```

The Box is not limited to software written by Softadastra.

An organization should be able to use it for compatible software it owns, purchases, commissions, or already operates.

A dedicated Box can provide a simpler experience because Softadastra can control more of the environment, including hardware, storage, connectivity, startup behavior, updates, supervision, and physical deployment.

An existing machine and a Softadastra Box therefore share the same fundamental model:

```text
existing machine + Softadastra = Host

Softadastra Box = Host
```

The hardware may evolve across generations.

The Host model remains the same.

## Building

Softadastra is written in C++20.

### With Vix.cpp

If Vix.cpp is installed:

```bash
git clone https://github.com/softadastra/softadastra.git
cd softadastra

vix build
vix tests
```

### With CMake

Softadastra can also be configured directly with CMake:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Vix.cpp may be used internally where its existing C++ primitives match Softadastra's infrastructure needs.

Hosted software does not need to use Vix.cpp.

## Project direction

Softadastra is intentionally built around one foundational abstraction:

```text
Host
```

New capabilities should be introduced because a real Host problem requires them, not simply because a technology is interesting.

The project prefers a small conceptual model and composition over accumulating independent infrastructure systems.

Future capabilities may include broader machine support, dedicated Box generations, improved local connectivity, remote access, and Host-to-Host communication.

These capabilities should continue to grow around the same Host model rather than creating a different architecture for every environment.

For the long-term mission, design boundaries, and project guardrails, see [MISSION.md](MISSION.md).

## License

Softadastra is licensed under the Apache License, Version 2.0.

See [LICENSE](LICENSE) for the full license text.
