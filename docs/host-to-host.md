# Host-to-Host

Host-to-Host describes how multiple Softadastra Hosts may communicate and cooperate.

This capability is not required for a Host to function.

A Host must remain useful on its own:

```text
Host
  ↓
Software
```

Host-to-Host begins only when another Host becomes useful.

```text
Host A
  ↕
Host B
```

The purpose is not to turn Softadastra into a distributed system by default. It is to make communication between independent Hosts possible when a real use case requires it.

## Independent Hosts first

Every Host keeps its own responsibility.

A Host runs and supervises the software registered on that machine.

```text
Host A
  └── Software A

Host B
  └── Software B
```

Neither Host should require the other one to boot, run local software, or provide local access.

If communication disappears, each Host should continue providing the capabilities that do not depend on the other Host.

This property is especially important on networks where connectivity may be intermittent.

## Why Hosts communicate

Host-to-Host should exist for concrete infrastructure needs.

Examples may include:

- reaching software hosted on another Host
- exchanging Host information
- coordinating operations between trusted Hosts
- connecting machines across separate local networks
- operating across intermittent connections

These cases should be built from a common Host communication model rather than from a different subsystem for every feature.

## Host identity

Before two Hosts can trust each other, each Host needs an identity.

Conceptually:

```text
Host
  ↓
HostIdentity
```

The identity belongs to the Host, not to an individual application running on it.

It can later support capabilities such as:

- identifying a remote Host
- verifying that the same Host is reconnecting
- authenticating communication
- establishing trusted relationships

An IP address is not a Host identity.

Addresses can change. A Host identity should remain stable across ordinary network changes.

## Transport

The transport carries communication between Hosts.

Conceptually:

```text
Host A
   ↓
Host transport
   ↓
Network
   ↓
Host transport
   ↓
Host B
```

The transport is an implementation detail below the Host-to-Host contract.

A first implementation may use TCP. Other transports may become useful later.

Applications should not need to know which transport Softadastra uses between Hosts.

## Security boundary

Communication between Hosts must be treated differently from local control.

Local control can rely on operating-system boundaries because the CLI and Host are on the same machine.

Host-to-Host communication crosses a network.

```text
Local control

CLI
 ↓
same machine
 ↓
Host
```

```text
Host-to-Host

Host A
 ↓
network
 ↓
Host B
```

A network connection must not automatically be considered trusted.

Before Host-to-Host is suitable for communication across untrusted networks, the system needs:

- Host authentication
- encryption
- peer verification
- authorization
- credential lifecycle
- failure handling

A raw TCP connection alone is not sufficient.

## Trust

Two Hosts should not become trusted merely because they can reach each other.

The intended relationship is closer to:

```text
Host A
  ↓
recognizes and trusts
  ↓
Host B
```

Trust should be explicit and tied to Host identity.

The exact user experience for establishing trust is not fixed yet.

It should remain simple enough that a user does not need to understand certificates, keys, or transport internals to connect two Hosts safely.

## Reachability

A Host may have different ways to reach another Host.

Examples include:

```text
same local network
internet connection
relay
private network
intermittent link
```

These are ways of reaching a Host, not different kinds of Hosts.

The higher-level relationship should remain:

```text
Host A
  ↕
Host B
```

Softadastra should avoid exposing unnecessary network topology to hosted software.

## Local networks

The simplest Host-to-Host case is two Hosts on the same local network.

```text
Local network

Host A
  ↕
Host B
```

A future discovery mechanism may allow Hosts to find each other locally.

Discovery alone must not establish trust.

Finding a Host and trusting a Host are separate operations.

## Across the Internet

Hosts may also need to communicate across different networks.

Direct inbound connectivity cannot always be assumed because of:

- NAT
- firewalls
- changing public addresses
- mobile networks
- networks controlled by third parties

A Host may therefore establish an outbound connection to infrastructure that helps another authorized Host reach it.

Conceptually:

```text
Host A
   ↓
outbound connection
   ↓
remote infrastructure
   ↑
outbound connection
   ↑
Host B
```

This infrastructure should extend communication between Hosts without becoming necessary for local operation.

## Intermittent connectivity

Host-to-Host should not assume a permanent network connection.

A useful model is:

```text
connected
   ↓
exchange what is possible

disconnected
   ↓
Hosts continue independently

connected again
   ↓
communication resumes
```

Not every operation can automatically survive disconnection.

Capabilities that require synchronization must define what happens when the remote Host is unavailable.

Softadastra should make these boundaries explicit rather than pretending the network is always reliable.

## Software reachability between Hosts

A future capability may allow software on one Host to be reached through another trusted Host.

Conceptually:

```text
User
 ↓
Host A
 ↓
Host B
 ↓
Software
```

This should not require the hosted Software to implement a Softadastra-specific protocol.

The Software still exposes its normal Access points.

Softadastra handles the infrastructure used to reach them.

## Coordination

Some future operations may involve more than one Host.

For example:

```text
Host A
  ↕
Host B
  ↕
Host C
```

This does not mean that every operation should become distributed.

Each Host should remain independently understandable.

Coordination should be introduced only when a capability cannot reasonably be implemented by one Host alone.

## What Host-to-Host is not

Host-to-Host is not intended to introduce:

- a mandatory cluster
- a mandatory central controller
- a new application framework
- a distributed database for every application
- automatic application semantics
- permanent Internet dependence

A single Host remains a complete Softadastra Host.

```text
one machine
    +
Softadastra
    ↓
complete Host
```

Multiple Hosts add new possibilities. They do not redefine the basic model.

## Relationship with applications

Hosted software remains opaque.

A Host-to-Host system should not need to know whether an application is:

- a frontend
- a backend
- a database
- a game
- an inventory system
- an API
- a desktop application

Softadastra sees hosted Software and its Access points.

Any higher-level relationship between applications belongs to the applications unless Softadastra needs a general infrastructure concept to represent it.

## Failure model

Network failures are normal.

Host-to-Host must eventually handle cases such as:

- remote Host unavailable
- connection interrupted
- Host restarted
- address changed
- identity rejected
- authentication failed
- operation timed out
- connection restored later

A failure to reach another Host must not corrupt the local Host state.

Local software should continue running whenever the failed remote operation is not required for that software.

## Design direction

Host-to-Host should grow from a small set of concepts.

The likely foundation is:

```text
HostIdentity
     ↓
Trust
     ↓
Transport
     ↓
Reachability
     ↓
Host operation
```

These concepts are not all final APIs.

They describe the separation of responsibilities that should guide future work.

A new Host-to-Host feature should first ask whether it can be represented with these concepts before introducing another subsystem.

## Current status

Host-to-Host is still an evolving part of Softadastra.

Some lower-level pieces already exist or are being explored, including Host identity, remote reachability, and Host transport.

The complete trust, authentication, authorization, and remote-operation model is not finished.

Host-to-Host should therefore not yet be treated as a production-ready public Internet communication system.

The immediate priority remains a reliable standalone Host.

Host-to-Host should advance only when the Host foundation and the product need justify the next capability.
