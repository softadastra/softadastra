# Softadastra Roadmap

This roadmap describes the product capabilities Softadastra is building toward.

It is intentionally organized around observable Host capabilities rather than internal classes, files, or implementation details.

The roadmap may evolve as real usage reveals better solutions, but the foundational direction remains:

```text
Machine + Softadastra = Host
```

Softadastra should grow by solving real Host problems, not by accumulating infrastructure subsystems without a demonstrated need.

## Phase 1: Host Foundation

### Goal

Turn a supported machine into a functioning Softadastra Host.

```text
supported machine
       +
Softadastra
       =
working Host
```

The first foundation must establish the minimum infrastructure required for the Host model to exist in practice.

### Capabilities

- establish the Host model;
- provide real platform integration;
- execute real processes on a supported machine;
- maintain Host-owned infrastructure state;
- provide local Host control;
- expose clear lifecycle state;
- detect basic execution failures;
- provide clear diagnostics;
- provide a usable command-line interface.

### Completion criteria

A supported machine can run Softadastra as a real Host rather than only as an in-memory model or test environment.

The Host can be controlled locally and can report meaningful infrastructure state.

---

## Phase 2: Software Hosting

### Goal

Run real existing software without requiring a Softadastra SDK or application framework.

```text
existing software
       |
       v
    register
       |
       v
     start
       |
       v
     status
       |
       v
      stop
```

Softadastra must treat hosted software as externally owned software rather than as a Softadastra application type.

### Capabilities

- register software with a Host;
- describe only the infrastructure information required to launch it;
- associate software with its execution lifecycle;
- start software;
- stop software;
- inspect software lifecycle state;
- detect process termination;
- report startup and runtime failures;
- manage more than one software entry;
- preserve Host-owned software metadata.

### Constraints

Software hosting must not require:

- a Softadastra SDK;
- a specific programming language;
- a specific framework;
- a specific database;
- HTTP;
- WebSocket;
- a proprietary Softadastra application protocol.

### Completion criteria

A real third-party or user-owned program can be registered, started, inspected, stopped, and managed by a Host without being redesigned around Softadastra.

---

## Phase 3: Local Access

### Goal

Make software hosted on a nearby Host simple to reach from local devices.

```text
phone
   \
laptop ---> Host ---> software
   /
tablet
```

Users should not need to understand unnecessary network infrastructure simply to access software running near them.

### Capabilities

Depending on the capabilities of the machine and network environment, this phase may include:

- local Host addressing;
- local reachability;
- local Host discovery;
- use of existing Wi-Fi networks;
- use of Ethernet networks;
- local naming where appropriate;
- clear connectivity diagnostics;
- simplified access information;
- Host-provided local connectivity where supported.

### Constraints

Local access must remain an infrastructure concern.

Softadastra must not classify hosted software by application protocol merely to provide local connectivity.

### Completion criteria

A user on a supported local network can discover or obtain a simple way to reach software running on the Host without manually configuring unnecessary infrastructure details.

---

## Phase 4: Host Persistence

### Goal

Make the Host persistent infrastructure rather than a terminal-bound process.

```text
terminal closes
     |
     v
Host continues
```

```text
machine restarts
     |
     v
Host returns
```

```text
software exits unexpectedly
     |
     v
Host detects it
```

### Capabilities

- run Softadastra independently of an interactive terminal;
- integrate with supported operating-system service mechanisms;
- restore Host operation after machine restart;
- maintain Host-owned state across restarts;
- detect software process termination;
- expose meaningful failure state;
- support controlled Host shutdown;
- provide basic recovery behavior where justified.

### Completion criteria

Closing the terminal does not stop the Host.

After a supported machine restart, the Host can return to an operational state with its infrastructure metadata intact.

---

## Phase 5: Remote Access

### Goal

Extend a Host beyond the local network without making local operation dependent on continuous Internet access.

```text
local users
    |
    v
   Host
    ^
    |
 Internet
    |
remote user
```

### Capabilities

This phase may include:

- secure remote Host access;
- remote administration;
- Host identity;
- authenticated Host control;
- encrypted connectivity;
- remote reachability through changing network environments;
- optional relay infrastructure when direct connectivity is unavailable;
- clear distinction between local and remote availability.

### Principle

```text
Local = foundation
Internet = extension
```

Loss of Internet access should not unnecessarily prevent local Host operation.

### Completion criteria

An authorized remote user can securely reach or administer a Host when Internet connectivity is available, while local capabilities remain independent where possible.

---

## Phase 6: Softadastra Box

### Goal

Provide the Host model as a dedicated physical product.

```text
dedicated machine
       +
Softadastra
       =
Softadastra Box
       =
Host
```

The first Box does not require custom silicon or custom hardware manufacturing.

It can begin with existing hardware that provides the capabilities required for a dependable dedicated Host.

### Early Box direction

An initial Box may combine:

```text
mini-PC
+
storage
+
network connectivity
+
supported operating environment
+
Softadastra
```

### Capabilities

A dedicated Box should progressively improve:

- installation simplicity;
- startup behavior;
- local connectivity;
- continuous operation;
- storage reliability;
- update handling;
- diagnostics;
- low-maintenance operation;
- physical deployment;
- energy efficiency;
- security;
- serviceability.

### Software independence

A Softadastra Box must remain able to host compatible software that:

- belongs to the customer;
- comes from another software company;
- was developed by an independent developer;
- already existed before Softadastra.

The Box must not become a closed platform restricted to Softadastra-developed applications.

### Completion criteria

A user or organization can deploy a dedicated Softadastra Host with substantially less infrastructure setup than a general-purpose machine.

### First Box implementation status

- [x] Define minimum and recommended hardware needs
- [x] Define a vendor-neutral x86_64 mini-PC profile
- [x] Prepare reproducible Debian/Ubuntu provisioning
- [x] Install the Host through a systemd unit
- [x] Enable Host startup at boot
- [x] Provide local status, access and connectivity commands
- [x] Exercise lifecycle, network availability and reboot restoration in tests
- [x] Exercise independent third-party process categories in tests
- [ ] Build the first physical prototype

---

## Phase 7: Host-to-Host

### Goal

Allow multiple Hosts to communicate and compose without replacing the Host abstraction with a larger speculative computing model.

```text
Host A <-> Host B <-> Host C
```

This phase should begin only when real Host usage demonstrates problems that require communication between Hosts.

### Possible capabilities

Real use cases may eventually justify:

- Host identity;
- secure Host-to-Host connections;
- Host discovery;
- direct communication;
- relays;
- local Host networks;
- regional Host networks;
- connectivity across unreliable networks;
- resilient communication paths;
- controlled sharing of Host capabilities.

### Constraints

Softadastra must not pre-build:

- a global distributed scheduler;
- a universal distributed store;
- a global orchestration fabric;
- a speculative worldwide mesh;
- another fundamental runtime abstraction;

unless real Host requirements prove that such a capability is necessary.

### Completion criteria

Two or more Hosts can securely communicate to solve a demonstrated infrastructure problem while remaining independently useful Hosts.

### Phase 10 implementation status

- [x] Reuse persistent Host identity as public peer identity
- [ ] Implement TLS 1.3 direct Host-to-Host connection
- [ ] Implement explicit-address peer reachability
- [ ] Implement non-sensitive peer communication
- [ ] Test three independent Hosts on a LAN
- [ ] Test peer loss and reconnection
- [x] Study NAT honestly
- [x] Study relay necessity without implementing one
- [x] Study regional independent-Host use
- [x] Keep the architecture free of global coordination

---

## Beyond the initial roadmap

Long-term Softadastra development may expand into new machine types, connectivity environments, hardware generations, and Host compositions.

Those developments should continue to satisfy the same architectural test:

```text
Does this solve a real Host problem?
             |
        +----+----+
        |         |
       yes        no
        |         |
        v         v
consider it    do not add it
```

Before introducing a major capability, Softadastra should ask:

1. Does it solve a real problem at the Host infrastructure boundary?
2. Can the existing Host model represent it?
3. Does it force hosted software to adopt a Softadastra architecture?
4. Is it required by current usage or only technically interesting?
5. Can the infrastructure complexity be hidden safely from the user?
6. Are we hiding a genuine incompatibility behind unnecessary abstraction?

## Roadmap principle

The roadmap is not a commitment to build every possible infrastructure technology.

It defines a direction of increasing Host usefulness:

```text
Host foundation
      |
      v
real software hosting
      |
      v
local access
      |
      v
persistent Host
      |
      v
remote access
      |
      v
dedicated Box
      |
      v
Host-to-Host
```

Each phase should make the existing Host more useful.

A new phase should not require redefining what Softadastra is.

For the foundational mission and design guardrails, see [MISSION.md](MISSION.md).
