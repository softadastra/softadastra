# Softadastra Architecture

Softadastra provides a simple hosting layer for existing software.

The architecture is built around a small number of concepts. The software being hosted remains independent from Softadastra and does not need to adopt a Softadastra framework, SDK, database, authentication system, programming language, or application architecture.

## The basic model

```text
Machine + Softadastra = Host
```

A Host runs software, keeps track of it, and makes its declared access points reachable.

```text
Machine
   +
Softadastra
   ↓
Host
   ↓
Software
   ↓
Access
```

The architecture deliberately stops there unless another concept becomes necessary.

## Host

A Host is a machine running Softadastra.

The machine may be:

- a personal computer
- a mini-PC
- a server
- a virtual machine
- a dedicated Softadastra Box

The Host has three fundamental responsibilities:

```text
run
reach
available
```

In practical terms, it must:

- run registered software
- expose the access declared by that software
- supervise its execution and availability

A Host does not need to understand what the application does internally.

## Software

Software is a program managed by a Host.

For Softadastra, hosted software is opaque.

A program may be written in C++, JavaScript, PHP, Python, Rust, Go, Java, or another language. It may use any database, authentication system, framework, protocol, or internal architecture that its environment supports.

Softadastra does not introduce another application model on top of it.

A simple project may describe itself with:

```toml
name = "example"
command = "./example"
access = "http:8080"
```

The important information for the Host is the command to supervise and the access points declared by the software.

## Command

The command is the command provided by the user.

For example:

```toml
command = "php -S 0.0.0.0:8080"
```

Softadastra may use platform-specific mechanisms internally to execute it, but the declared command remains unchanged.

On Linux, a command may be executed through the system shell.

On Windows, a command may be executed through `cmd.exe`.

Those are implementation details of the Host. They are not part of the application's architecture.

## Access

An Access describes a point through which software can be reached.

For example:

```toml
[[access]]
protocol = "http"
port = 8080
```

A program may expose more than one access point:

```toml
[[access]]
protocol = "http"
port = 8080

[[access]]
protocol = "ws"
port = 9090
```

Access does not mean that Softadastra owns or interprets the application's protocol.

It describes where the application is reachable.

The legacy single-access form remains readable:

```toml
access = "http:8080"
```

## Project configuration

A project is described by `softadastra.toml`.

The project file is the source of truth for project configuration.

The Host also maintains operational state for registered software. That state exists for supervision and must not replace the project configuration.

The two responsibilities are different:

```text
softadastra.toml
    ↓
project configuration

Host state
    ↓
runtime and supervision state
```

## Local control

The command-line interface communicates with the local Host through a platform-specific control channel.

The transport is an implementation detail.

Conceptually:

```text
CLI
 ↓
Local Control
 ↓
Host
```

Linux and Windows may use different native mechanisms while preserving the same control model.

The local control channel is not an application protocol and is not exposed as a requirement to hosted software.

## Process supervision

The Host supervises real operating-system processes.

The lifecycle is conceptually:

```text
registered
    ↓
starting
    ↓
running
    ↓
stopping
    ↓
stopped
```

Failures are also observable.

The Host is responsible for operations such as:

- starting software
- stopping software
- restarting software
- observing its state
- capturing its output
- detecting failed launches
- restoring software that should be running after a Host restart

The process implementation differs by operating system, but the Software lifecycle should remain consistent.

## Local operation

Local operation is fundamental to Softadastra.

A Host should not require Internet access simply to run software on the local machine or local network.

```text
Software
   ↓
Host
   ↓
Local access
```

Internet connectivity may extend the Host with remote capabilities, but it is not part of the basic execution contract.

If the hosted software itself depends on an Internet service, that dependency still belongs to the software.

Softadastra cannot make an external dependency local automatically.

## Local reachability

A Host may use the network already available on the machine.

On supported systems, a dedicated Host may also manage additional local networking capabilities.

These responsibilities can include:

```text
LocalAccess
LocalDns
LocalGateway
ManagedNetwork
```

They are infrastructure capabilities of the Host, not application requirements.

A normal application should not need to know whether its access is being exposed through an existing LAN, a dedicated Box network, or another supported local configuration.

## Remote reachability

Remote access extends a Host beyond its local network.

The architectural relationship remains:

```text
Software
   ↓
Host
   ├── local access
   └── remote access
```

Remote connectivity must not become a prerequisite for local operation.

Security, Host identity, authentication, encryption, and authorization are required before remote access can be treated as suitable for public Internet use.

## Softadastra Box

A Softadastra Box is a dedicated Host.

It does not introduce a second application platform.

```text
Personal computer + Softadastra
              ↓
             Host

Dedicated machine + Softadastra
              ↓
             Host
              ↓
             Box
```

The difference is operational.

A Box can be prepared specifically for continuous hosting, controlled networking, recovery after power loss, and simplified deployment.

Software should not need a different architecture merely because it runs on a Box.

## Platform boundary

Softadastra supports multiple operating systems through a platform boundary.

Higher-level components should depend on platform contracts rather than operating-system APIs directly.

Conceptually:

```text
SoftwareManager
HostService
Control
    ↓
Platform contracts
    ↓
Native implementation
    ├── Linux
    └── Windows
```

Platform-specific behavior belongs as low in the architecture as practical.

This keeps the Host model independent from details such as POSIX processes, Windows Job Objects, Unix sockets, Named Pipes, or Winsock.

## What Softadastra does not own

The boundary around hosted software is intentional.

Softadastra does not own the application's:

- source code
- programming language
- framework
- database
- authentication
- business logic
- frontend
- backend
- internal services
- application protocol

Those belong to the software.

Softadastra owns the hosting responsibilities around it.

```text
Application internals
        │
        │ opaque
        ▼
+-----------------------+
|       Software        |
+-----------------------+
           │
           ▼
+-----------------------+
|         Host          |
|                       |
| execution             |
| supervision           |
| access                |
| availability          |
+-----------------------+
```

## Compatibility

Infrastructure independence does not mean that every binary can execute on every machine.

Software must still be compatible with the operating system, CPU architecture, runtime dependencies, and other requirements of the Host.

Softadastra should detect failures clearly where possible, but it does not emulate arbitrary platforms.

The architectural promise is narrower:

> Softadastra does not require an additional application architecture in order to host software.

## Design direction

New capabilities should be introduced from a small set of reusable concepts rather than by adding a specialized subsystem for every application type.

The architecture should remain understandable from the outside:

```text
Host
  ↓
Software
  ↓
Access
```

Internal complexity may grow as Softadastra supports more platforms, networking models, supervision mechanisms, and hardware profiles.

That complexity should remain behind these concepts whenever possible.
