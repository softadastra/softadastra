# Softadastra

Softadastra turns a machine into a Host for running software.

```text
Machine + Softadastra = Host
```

A Host runs software, keeps it available, and makes its declared access points reachable.

The software itself stays independent. It does not need a Softadastra framework, SDK, database, authentication system, programming language, or application architecture.

## Why Softadastra

Running software is often only the beginning.

It also needs a machine, a process lifecycle, logs, networking, recovery, and a way to remain available.

These are infrastructure concerns. They should not have to become part of the software's architecture.

Softadastra keeps the boundary simple:

```text
Software
    ↓
Softadastra Host
    ↓
Machine
```

A project tells the Host two essential things:

```text
what to run
how to reach it
```

The Host handles the infrastructure around that execution.

## Install

### Linux

```bash
curl -fsSL https://softadastra.com/install.sh | bash
```

### Windows

Download Softadastra for Windows from:

https://softadastra.com/download

Other installation methods, packages, and portable builds are available on the download page.

## Run your first software

Inside a project, describe how the software starts and how it can be reached:

```bash
softadastra init my-app \
  --command "node server.js" \
  --access http:8080
```

This creates `softadastra.toml`:

```toml
name = "my-app"
command = "node server.js"
access = "http:8080"
```

Run it:

```bash
softadastra run
```

Check its state:

```bash
softadastra status
```

Read its logs:

```bash
softadastra logs
```

Allow local access when needed:

```bash
softadastra access allow
```

Then inspect how it can be reached:

```bash
softadastra access
```

Stop it:

```bash
softadastra stop
```

The command is not limited to an executable file.

It can start any software compatible with the Host:

```toml
command = "./server"
```

```toml
command = "python3 app.py"
```

```toml
command = "node server.js"
```

```toml
command = "php -S 0.0.0.0:8080"
```

Softadastra does not classify software by language, framework, runtime, or application type.

It only needs to know how the software starts and how it can be reached.

## Host, Software, Access

Softadastra is built around a small set of public concepts:

```text
Host
├── Software
└── Access
```

### Host

A machine running Softadastra becomes a Host.

A Host is responsible for:

```text
Execution
Reachability
Availability
```

The machine can be a personal computer, mini-PC, server, virtual machine, or dedicated Softadastra Box.

A Box is not a separate application platform. It is a Host running on dedicated hardware.

### Software

Software is what the Host runs and supervises.

Softadastra treats it as opaque.

It does not need to understand:

- its programming language
- its framework
- its database
- its authentication system
- its internal services
- its application architecture

Software can be managed directly from its project directory:

```bash
softadastra start
softadastra stop
softadastra restart
softadastra status
```

Registered software can also be addressed by name:

```bash
softadastra status my-app
```

Run:

```bash
softadastra --help
```

to explore the complete command-line interface.

### Access

Access describes how software can be reached.

A project can declare one access point:

```toml
name = "example"
command = "python3 app.py"
access = "http:8080"
```

Or several:

```toml
name = "example"
command = "node server.js"

[[access]]
protocol = "http"
port = 8080

[[access]]
protocol = "ws"
port = 9090
```

Access describes the boundary between the software and the Host.

It does not describe the internal architecture of the software.

## Local first

A local Host should not require Internet access just to run local software.

```text
Software
    ↓
Host
    ├── Local access
    └── Remote access, optional
```

If Internet connectivity disappears, local operation can continue as long as the hosted software itself does not depend on an external Internet service.

Remote access extends the Host. It does not define the Host.

## One Host model

Softadastra keeps the same Host model across different machines.

```text
Personal computer
Mini-PC
Server
Virtual machine
Softadastra Box
        ↓
       Host
```

Platform-specific mechanisms remain below this model.

Process management, networking, local control, and system integration may differ between operating systems without changing how hosted software is represented to the user.

## Web interface

Softadastra also provides a local Web interface:

```bash
softadastra ui
```

The CLI and Web interface control the same Host.

They are two interfaces to the same system, not separate hosting platforms.

## Boundaries

Softadastra hosts software. It does not define the software.

Hosted software does not need to adopt:

- a Softadastra application framework
- a Softadastra SDK
- a specific programming language
- a specific database
- a specific authentication system
- a frontend/backend architecture
- a specific application protocol

Softadastra also does not make incompatible software portable automatically.

The software must still be compatible with the operating system, CPU architecture, runtime, and dependencies available on the Host.

## Mission

Softadastra is built around a broader question:

> Can software be separated from the infrastructure that executes it?

The project does not try to hide the physical limits of computing.

It tries to prevent infrastructure choices from becoming unnecessary application architecture.

See [`MISSION.md`](MISSION.md).

## Learn more

- [Architecture](docs/architecture.md)
- [Mission](MISSION.md)
- [Roadmap](ROADMAP.md)
- [Host-to-Host](docs/host-to-host.md)

## Development

Softadastra is written in C++ and uses Vix.cpp internally.

Vix.cpp is an implementation dependency of Softadastra. Software hosted by Softadastra does not need to use Vix.cpp.

The project uses unit, integration, and end-to-end tests across supported platforms.

## License

Softadastra is licensed under the Apache License 2.0.

See [`LICENSE`](LICENSE).
