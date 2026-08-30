# Softadastra Mission

Softadastra exists to make computing infrastructure-independent.

The question behind the project is simple:

> Can software be separated from the infrastructure that executes it?

Today, software often becomes tightly connected to the environment around it. Deployment choices, operating systems, networking, hosting providers, process supervision, and machine configuration can become part of the application itself.

Softadastra explores a different boundary.

```text
Software
    ↓
Softadastra
    ↓
Infrastructure
```

The software should describe what it needs to accomplish, while Softadastra handles more of the infrastructure required to execute it.

## Software should remain software

Softadastra does not ask applications to become Softadastra applications.

A program should not need to adopt a specific:

- framework
- SDK
- programming language
- database
- authentication system
- protocol
- application architecture

A C++ program can remain a C++ program.

A PHP application can remain a PHP application.

A JavaScript service can remain a JavaScript service.

Softadastra should understand only what is necessary to host them.

## The Host

The first practical expression of this mission is the Softadastra Host.

```text
Machine + Softadastra = Host
```

A Host has a small set of responsibilities:

```text
run
reach
available
```

It runs software, makes it reachable, and keeps track of its availability.

The internal infrastructure needed to provide those capabilities may differ between Linux, Windows, a personal computer, a server, or a dedicated Box.

The software should not need a different architecture for each one.

## Local operation matters

Infrastructure independence also means avoiding unnecessary dependence on the Internet.

A local application should be able to run locally when the Internet is unavailable, provided the application itself does not depend on an external service.

```text
Software
    ↓
Host
    ├── local operation
    └── remote capabilities when available
```

Remote access should extend a Host, not define whether the Host can function.

This matters in places where connectivity is unreliable, expensive, intermittent, or simply unnecessary for the work being done.

## A Box is still a Host

Softadastra may run on a machine the user already owns.

It may also run on dedicated hardware.

```text
Personal machine
      +
  Softadastra
      ↓
     Host
```

```text
Dedicated machine
       +
   Softadastra
       ↓
      Host
       ↓
      Box
```

A Softadastra Box is not a separate software platform.

It is a Host placed on hardware intended to run continuously and with less administration.

The same software should not need to be redesigned simply because the Host is now dedicated hardware.

## Infrastructure independence has limits

Softadastra does not mean that every binary can run on every machine.

Software still depends on real constraints such as:

- operating system
- CPU architecture
- runtime dependencies
- hardware capabilities
- external services

Softadastra should not pretend those constraints do not exist.

The goal is to remove infrastructure coupling that does not need to belong to the application.

## Build from a few concepts

Softadastra follows a simple design principle:

> Do not build every solution. Find the few concepts from which solutions can be built.

The project should not grow by creating a specialized subsystem for every new application type or deployment case.

When a new problem appears, the first question should be whether the existing concepts can already represent it.

The architecture should remain small enough to understand:

```text
Host
  ↓
Software
  ↓
Access
```

More capabilities may appear around those concepts, but the application should continue to see a simple hosting model.

## Simplicity belongs at the interface

Softadastra may require complex internals to support different operating systems, networking models, hardware, process managers, or remote communication.

That complexity should stay inside Softadastra.

A user should not need to understand those mechanisms before running software.

The desired experience is closer to:

```text
install Softadastra
        ↓
the machine becomes a Host
        ↓
add software
        ↓
run it
        ↓
reach it
```

If using the system requires understanding its internal architecture first, the interface still needs work.

## Long-term direction

Softadastra begins with one Host running software on one machine.

From there, the same ideas can extend toward:

- dedicated Softadastra Boxes
- secure remote access
- resource control
- multiple executions
- communication between Hosts
- larger groups of cooperating machines

These are not separate missions.

They all come from the same question:

> How much infrastructure can be moved behind a small, stable boundary while software remains independent of it?

That is the problem Softadastra is trying to solve.
