# Softadastra

Softadastra is a simple hosting layer for software.

It turns a machine into a Host that can run software, supervise it, and make its declared access points reachable.

```text
Machine + Softadastra = Host
```

The software itself stays independent. It does not need a Softadastra framework, SDK, database, authentication system, programming language, or application architecture.

## Why Softadastra

Running an application is often only the beginning.

The application also needs a machine, a process lifecycle, logs, networking, recovery, and some way to remain available.

These infrastructure concerns can become part of the application even when they have little to do with what the application actually does.

Softadastra explores a simpler boundary:

```text
Software
    ↓
Softadastra Host
    ↓
Machine
```

The software describes what should run and how it can be reached. The Host handles the infrastructure around that execution.

## A first application

Inside a project, tell Softadastra how the software is started and how it can be reached:

```bash
softadastra init my-app \
  --command "./my-app" \
  --access http:8080
```

This creates `softadastra.toml`:

```toml
name = "my-app"
command = "./my-app"
access = "http:8080"
```

Run the software:

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

Stop it:

```bash
softadastra stop
```

The command is opaque to Softadastra. It describes how the software is started, not what kind of software it is.

## Host

A machine running Softadastra becomes a Host.

The Host has three fundamental responsibilities:

```text
run
reach
available
```

It runs software, makes it reachable, and supervises its availability.

A Host can be:

- a personal computer
- a mini-PC
- a server
- a virtual machine
- a dedicated Softadastra Box

A Box is not a separate application platform. It is a Host running on dedicated hardware.

## Project configuration

A project is described by `softadastra.toml`.

A project can declare multiple access points:

```toml
name = "example"
command = "./example"

[[access]]
protocol = "http"
port = 8080

[[access]]
protocol = "ws"
port = 9090
```

The configuration describes the software to the Host without describing the internal architecture of the application.

## Commands

The command-line interface provides the main Host and software operations.

```text
softadastra init
softadastra run
softadastra register

softadastra start
softadastra stop
softadastra restart

softadastra status
softadastra info
softadastra list
softadastra remove

softadastra logs
softadastra access

softadastra connectivity
softadastra network
softadastra remote

softadastra ui
softadastra host
softadastra box
```

Most software commands can work in two ways.

From the current project:

```bash
softadastra start
```

Or by referring to registered software:

```bash
softadastra start my-app
```

Use:

```bash
softadastra --help
```

or the help for an individual command to inspect the available options.

## Local first

A local Host should not require Internet access just to run local software.

```text
Application
    ↓
Host
    ├── local access
    └── remote access, optional
```

If Internet disappears, local operation should remain possible as long as the application itself does not depend on an external Internet service.

Remote access extends the Host. It does not define the Host.

## Linux and Windows

Softadastra is designed for Linux and Windows.

The Host model remains the same while platform-specific implementation details stay below it.

For example, process management and local control may use different operating-system mechanisms without changing how hosted software is represented.

Not every capability is necessarily available on every platform at the same stage of development. Advanced managed networking is currently primarily a Linux and dedicated-Host concern.

## Web interface

Softadastra also provides a local Web interface:

```bash
softadastra ui
```

The interface provides a simpler view of applications, their state, configuration, lifecycle, and logs.

The CLI and Web interface control the same Host. They are not separate hosting systems.

## What Softadastra does not require

Hosted software does not need to adopt:

- a Softadastra application framework
- a Softadastra SDK
- a specific programming language
- a specific database
- a specific authentication system
- a frontend/backend architecture
- a specific application protocol

Softadastra also does not make incompatible software portable automatically.

A program must still be compatible with the operating system, CPU architecture, runtime, and dependencies available on the Host.

## Architecture

The public architecture is intentionally small:

```text
Host
  ↓
Software
  ↓
Access
```

More details are available in [`docs/architecture.md`](docs/architecture.md).

Host-to-Host work is documented separately in [`docs/host-to-host.md`](docs/host-to-host.md).

## Roadmap

Softadastra is being developed incrementally.

The current direction moves from a reliable Host toward local reachability, dedicated Boxes, secure remote access, resource control, and eventually cooperation between Hosts.

See [`ROADMAP.md`](ROADMAP.md).

## Mission

Softadastra is built around a broader question:

> Can software be separated from the infrastructure that executes it?

The project does not try to hide the physical limits of computing. It tries to prevent infrastructure choices from becoming unnecessary application architecture.

See [`MISSION.md`](MISSION.md).

## Development

Softadastra is written in C++ and uses Vix.cpp internally.

Vix.cpp is an implementation dependency of Softadastra. Applications hosted by Softadastra do not need to use Vix.cpp.

The project uses unit, integration, and end-to-end tests across Linux and Windows. Platform-specific mechanisms are kept behind common Host concepts whenever practical.

## Status

Softadastra is under active development.

The Host foundation is being stabilized before larger capabilities are added. Interfaces, package formats, and some system behavior may still change before later stable releases.

See [`ROADMAP.md`](ROADMAP.md) for the current development order.

## License

Softadastra is licensed under the Apache License 2.0.

See [`LICENSE`](LICENSE).
