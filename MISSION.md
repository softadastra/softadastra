# Softadastra

## Mission

> **Make computing infrastructure simple, resilient, and available where people need it.**

Softadastra builds infrastructure that allows software to run close to the people and organizations that need it, without forcing that software to adopt a Softadastra application architecture.

Softadastra does not exist to define how applications should be built.

Softadastra exists to make the infrastructure around software simpler.

The long-term technological direction is:

> **Turn suitable machines into simple, resilient Hosts for existing and future software.**

Products may change.

Hardware may change.

Interfaces may change.

Supported platforms may change.

The mission must remain recognizable.

---

## 1. The Host

The foundational Softadastra concept is the **Host**.

```text
Machine + Softadastra = Host
```

A Host is a supported machine on which Softadastra provides infrastructure capabilities for running software, making it reachable, and maintaining its availability within the real capabilities of the machine and its environment.

A Host may be:

- a user's existing computer;
- a desktop machine;
- a mini-PC;
- a server;
- a dedicated Softadastra Box;
- or another supported machine in the future.

A Host is not defined by one operating system, processor architecture, hardware vendor, programming language, application protocol, or deployment environment.

---

## 2. The software belongs to its owner

Softadastra manages infrastructure.

It does not own the internal architecture of the software running on that infrastructure.

```text
+--------------------------------------+
|               SOFTWARE               |
|                                      |
|  business logic                      |
|  programming language                |
|  databases                           |
|  authentication                      |
|  authorization                       |
|  users                               |
|  protocols                           |
|  internal services                   |
|  process topology                    |
|  dependencies                        |
|  data model                          |
|                                      |
|       owned by the software          |
+-------------------+------------------+
                    |
                    | hosted on
                    v
+--------------------------------------+
|                 HOST                 |
|                                      |
|  execution infrastructure            |
|  machine integration                 |
|  supervision                         |
|  connectivity                        |
|  reachability                        |
|  availability                        |
|                                      |
|       owned by Softadastra           |
+--------------------------------------+
```

The fundamental boundary is:

> **Softadastra owns the infrastructure boundary, not the software architecture.**

---

## 3. Software is opaque

Softadastra must not assume that hosted software follows a particular architecture.

Software may consist of:

- one native process;
- an interpreted program;
- multiple processes;
- a legacy system;
- a network service;
- a local service;
- HTTP and WebSocket together;
- a proprietary protocol;
- local databases;
- remote databases;
- no database at all;
- hardware integrations;
- internal components Softadastra does not understand.

That internal organization belongs to the software.

Softadastra should know only what is actually required to host the software correctly.

---

## 4. Software Independence Contract

Fundamental hosting SHALL NOT require:

- a Softadastra SDK;
- a Softadastra library inside the hosted software;
- a Softadastra application framework;
- a Softadastra database;
- a Softadastra authentication system;
- a Softadastra ORM;
- a particular programming language;
- a particular frontend architecture;
- a particular backend architecture;
- a particular database;
- HTTP;
- WebSocket;
- gRPC;
- a proprietary Softadastra application protocol;
- a particular source-code layout;
- a Softadastra business model.

Existing software should not require architectural redesign merely to become hostable by Softadastra.

The preferred direction is:

```text
adapt Softadastra to the software
```

not:

```text
redesign the software for Softadastra
```

When integration information is required, it should describe infrastructure reality rather than impose a programming model.

---

## 5. Compatibility Contract

Softadastra does not promise:

> Every software package runs on every Host.

The actual rule is simpler:

> **A Host runs software when the machine and its available capabilities satisfy the real requirements of that software.**

```text
software
   |
   v
can this Host support it?
   |
   +-- yes --> run
   |
   +-- no  --> explain clearly
```

Incompatibility may result from:

- processor architecture;
- missing runtime environment;
- unsupported operating-system capability;
- unavailable hardware;
- insufficient permissions;
- insufficient resources;
- another real machine-level limitation.

When software is incompatible with a Host, Softadastra should produce a clear explanation.

Softadastra should not accumulate unnecessary abstraction layers merely to claim universal compatibility.

The goal is not to accept anything.

The goal is to do supported things exceptionally well.

---

## 6. Platform Independence Contract

A Softadastra Host is not defined by an operating system.

The concept is not:

```text
Linux + Softadastra = Host
```

It is:

```text
Supported machine + Softadastra = Host
```

Operating-system support is an implementation capability.

A machine becomes a usable Host only when Softadastra actually supports the capabilities required on that platform.

Conceptually, over time:

```text
personal computer   -> Host
server              -> Host
mini-PC             -> Host
Softadastra Box     -> Host
future machine      -> Host
```

The company direction must not become tied to the first platform used to implement Softadastra.

---

## 7. Host responsibilities

A Host has three broad responsibilities:

```text
HOST
 |
 +-- run
 |
 +-- make reachable
 |
 +-- remain available
```

### Run

A Host provides the infrastructure required to operate supported software on the machine.

### Make reachable

A Host should make hosted software reachable to the people or systems that are supposed to use it without forcing them to manage unnecessary infrastructure details.

### Remain available

A Host should provide the strongest practical continuity and resilience allowed by the machine and its environment.

Softadastra cannot violate physical limits.

If the Host loses power, computation stops.

If no long-distance communication medium exists, Softadastra cannot invent one.

Resilience means handling reality well, not pretending physical constraints do not exist.

---

## 8. Local is a foundation

Softadastra follows this principle:

> **Local is a foundation. Internet is an extension.**

A Host should remain useful locally without continuous Internet access whenever the hosted software itself allows local operation.

Softadastra must not introduce an unnecessary cloud dependency into software that can otherwise operate locally.

This does not mean that software with genuine remote dependencies becomes magically independent of them.

Softadastra respects the real requirements of the software.

---

## 9. Internet extends the Host

Internet may extend what a Host can do.

```text
local users
    |
    v
   Host
    |
    +--------- optional Internet --------- remote user
```

Future Host capabilities may include:

- remote access;
- remote administration;
- secure connectivity;
- backup;
- Host-to-Host communication;
- relays;
- other infrastructure extensions.

The local Host should not stop fulfilling its local role merely because one optional remote capability becomes unavailable.

---

## 10. Simplicity Contract

Softadastra should absorb infrastructure complexity when doing so is technically sound.

Users should not normally need to understand:

- IP addresses;
- port numbers;
- process identifiers;
- service supervisors;
- local DNS;
- network discovery;
- routing details;
- operating-system service configuration;
- Wi-Fi implementation details;
- infrastructure topology.

These mechanisms may exist internally.

They should not automatically become user concepts.

The principle is:

> **Infrastructure complexity belongs to Softadastra when Softadastra can reasonably manage it.**

When user action is genuinely required, Softadastra should explain what is needed clearly.

---

## 11. Clear failure over hidden complexity

Softadastra should prefer explicit failure to unpredictable behavior or uncontrolled complexity.

For example:

```text
This software cannot run on this Host.

Missing capability:
  required runtime is not available
```

or:

```text
Local access is unavailable on this Host.

Reason:
  no supported local connectivity is currently available
```

A real incompatibility should not be hidden behind increasingly complicated fallback systems.

---

## 12. Unix-inspired design principle

Softadastra follows a design principle inspired by Unix:

> **Do not build every solution. Find the few concepts from which solutions can be built.**

This does not mean minimizing code at any cost.

It means:

- keep the conceptual model small;
- keep responsibilities clear;
- prefer composition;
- avoid specialized abstractions when an existing concept is sufficient;
- introduce new concepts only when they solve a real problem that existing concepts cannot express cleanly.

The foundational concept is currently:

```text
Host
```

Everything else must earn its place.

---

## 13. Composition Contract

Softadastra should not create a different fundamental computing model for every environment.

Avoid:

```text
DesktopRuntime
BoxRuntime
EdgeRuntime
CloudRuntime
IndustrialRuntime
```

when those environments can remain:

```text
Host
```

For example:

```text
PC              = Host
Box             = Host
Server          = Host
Mini-PC         = Host
Future machine  = Host
```

Future communication should also prefer simple composition:

```text
Host <-> Host
```

before introducing a large new distributed-computing abstraction.

A new foundational concept must demonstrate why the Host model cannot represent the problem cleanly.

---

## 14. Capabilities grow from real problems

Softadastra should grow in this direction:

```text
real problem
    |
    v
required capability
    |
    v
implementation
```

Not:

```text
interesting technology
    |
    v
new subsystem
    |
    v
search for a use case
```

A scheduler, distributed store, SDK, framework, orchestration system, network fabric, or other major subsystem is not justified merely because it is technically interesting.

It must solve a real Host problem.

---

## 15. User-owned machines

Softadastra software can turn an already-owned supported machine into a Host.

```text
Your machine
     +
Softadastra
     =
Host
```

This is the most accessible way to use Softadastra.

The initial interface may be a CLI.

The CLI is not the mission.

It is an interface to the Host.

Closing the terminal should not necessarily stop the Host or the software it maintains.

---

## 16. Softadastra Box

A **Softadastra Box** is a dedicated Host supplied as a product.

```text
Dedicated machine
       +
Softadastra
       =
Softadastra Box
       =
Host
```

The Box is not limited to software written by Softadastra.

An organization should be able to use a Box for compatible software that it owns, purchased from another company, or commissioned from an independent developer.

The Box may provide a simpler experience because Softadastra can control more of the environment, including:

- hardware;
- storage;
- connectivity;
- startup behavior;
- updates;
- supervision;
- network setup;
- physical deployment.

The Box remains a Host.

It is not a separate computing model.

---

## 17. Box generations

The first Box does not need to represent the final hardware vision.

An early Box may use:

```text
existing mini-PC
+
storage
+
connectivity
+
supported system
+
Softadastra
```

Future generations may improve:

- reliability;
- power efficiency;
- connectivity;
- storage;
- enclosure design;
- thermal design;
- security;
- manufacturing;
- serviceability;
- dedicated hardware integration.

The concept remains:

```text
Box V1  = Host
Box V2  = Host
Box V10 = Host
```

Products evolve.

The direction remains stable.

---

## 18. Existing and third-party software

Softadastra is not restricted to software developed by Softadastra.

Hosted software may come from:

- the organization using the Host;
- Softadastra;
- another software company;
- an independent developer;
- an integrator;
- a previous vendor;
- an existing legacy installation.

The origin of the software does not change the role of the Host.

Softadastra provides infrastructure.

The software keeps its own architecture.

---

## 19. Connectivity Contract

Communication always requires a real communication medium.

Softadastra does not create physical communication from nothing.

A Host may use or provide supported connectivity such as:

- an existing Wi-Fi network;
- Ethernet;
- a Host-provided Wi-Fi access point;
- other supported technologies in the future.

The technology may vary.

The concept remains:

```text
Host
 |
 v
software reachable
```

The user experience should remain as simple as the underlying environment allows.

A future dedicated Box may, for example, provide an experience approaching:

```text
power on
   |
   v
scan
   |
   v
use
```

while Softadastra handles the infrastructure underneath.

---

## 20. Host-to-Host future

Softadastra should remain open to a future in which Hosts communicate directly.

```text
Host <-> Host <-> Host
```

This may eventually support:

- secure Host-to-Host connectivity;
- relays;
- local or regional networks;
- resilient communication;
- infrastructure in areas with limited Internet access;
- other forms of distributed local infrastructure.

This is a future possibility, not a requirement for the first version.

Softadastra must not build a speculative global network before real Host usage requires it.

Current architecture should simply avoid unnecessary decisions that make future Host-to-Host evolution impossible.

---

## 21. Global direction

Softadastra may begin where infrastructure problems are particularly painful.

Examples include environments where:

- Internet is expensive;
- Internet is unreliable;
- outages are frequent;
- server administration is difficult;
- organizations still need dependable software.

These conditions do not limit Softadastra geographically.

The Host model may also provide value through:

- resilience;
- autonomy;
- privacy;
- local control;
- lower complexity;
- lower latency;
- lower infrastructure cost;
- industrial use;
- remote sites;
- private networks;
- temporary installations.

The initial pain does not define the final market boundary.

---

## 22. What Softadastra is not

Softadastra is not fundamentally:

- an application framework;
- a library that every hosted program must embed;
- a mandatory SDK;
- a programming language;
- a database;
- an application authentication provider;
- an ORM;
- a frontend framework;
- a backend framework;
- an inventory application;
- an ERP;
- a Docker clone;
- a Kubernetes clone;
- a universal cloud replacement;
- a universal emulator;
- a particular operating system;
- a Linux-only technology;
- a distributed-computing project by default.

Softadastra may use existing technologies internally.

Internal implementation choices do not redefine the company mission.

---

## 23. Direction Guardrails

These rules exist specifically to prevent fragmentation.

### Guardrail 1: Application architecture

Softadastra SHALL NOT require hosted software to adopt a Softadastra application architecture.

### Guardrail 2: SDK

A Softadastra SDK SHALL NOT be mandatory for fundamental hosting.

### Guardrail 3: Programming language

The Host model SHALL NOT depend on the programming language used by hosted software.

### Guardrail 4: Protocols

Softadastra SHALL NOT require one universal application protocol.

### Guardrail 5: Business data

Application business data SHALL NOT become Softadastra-owned data merely because the software is hosted.

### Guardrail 6: Authentication

Softadastra SHALL NOT prescribe the internal authentication architecture of hosted software.

### Guardrail 7: Platforms

PCs, Boxes, servers, and future supported machines SHOULD remain Hosts unless a genuinely different foundational abstraction is required.

### Guardrail 8: Compatibility

Softadastra SHALL NOT claim compatibility that has not actually been implemented and verified.

### Guardrail 9: Diagnostics

Unsupported software or Host capabilities SHALL produce clear diagnostics.

### Guardrail 10: Concepts

Every new foundational concept SHALL demonstrate why the Host abstraction is insufficient.

### Guardrail 11: Major subsystems

Every new major subsystem SHALL solve a real Host problem.

### Guardrail 12: Internet

Softadastra SHALL NOT require continuous Internet access for local Host operation unless the hosted software or requested capability actually requires it.

### Guardrail 13: Box

A Softadastra Box SHALL remain capable of hosting compatible customer-owned or third-party software.

### Guardrail 14: Product generations

A particular CLI, Box generation, operating system, or hardware implementation SHALL NOT redefine the company mission.

---

## 24. Decision test

Before adding a major feature, ask:

1. Does it solve a real problem at the Host infrastructure boundary?
2. Does it require hosted software to adopt a Softadastra architecture?
3. Can the requirement already be expressed through the Host?
4. Is it necessary now, or merely technically interesting?
5. Can Softadastra hide this infrastructure complexity safely?
6. Are we hiding a real incompatibility behind unnecessary abstractions?

If a capability does not belong to the Host infrastructure boundary, it should not automatically become part of Softadastra.

---

## 25. Present direction

The current direction is intentionally small:

```text
Supported machine
       +
Softadastra
       =
Host
```

The Host should progressively make it simpler to:

```text
run software
     +
make it reachable
     +
keep it available
```

The first versions should prove this model well.

They do not need to implement every possible future capability.

---

## 26. Long-term direction

Future capabilities may grow around the same concept:

```text
                         Host
                          |
          +---------------+---------------+
          |               |               |
     existing machine     Box        future machine
                          |
                    Host <-> Host
                          |
                  future capabilities
```

Softadastra should grow outward from a strong Host primitive instead of accumulating unrelated products and abstractions.

---

## 27. Long-term identity

In ten years, this sentence should still make sense:

> **Softadastra builds infrastructure that lets software run where people need it.**

It should remain true even if:

- the CLI changes;
- supported operating systems change;
- the first Box no longer exists;
- hardware generations evolve;
- Hosts communicate globally;
- new products appear.

---

## 28. Foundational equations

```text
Machine + Softadastra = Host
```

```text
Software owns its architecture.
```

```text
The Host owns its infrastructure.
```

```text
Local = foundation
Internet = extension
```

```text
Compatible = run
Incompatible = explain
```

```text
Real problem
    |
    v
required capability
    |
    v
implementation
```

Not:

```text
interesting technology
    |
    v
new subsystem
    |
    v
search for a purpose
```

And when composition is sufficient:

```text
Host + Host + Host
```

should be preferred over introducing another giant foundational abstraction.

---

## 29. Final principle

Softadastra must remain disciplined enough to become large without becoming scattered.

The goal is not to accumulate concepts.

The goal is to build a small number of strong concepts from which larger capabilities can emerge.

The foundational Softadastra concept is the **Host**.

Everything else must earn its place.
