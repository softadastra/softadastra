# Softadastra

**Software that keeps working when the internet is unstable.**

Softadastra helps organizations build applications that can save data locally, continue working offline, and synchronize when the connection comes back.

<p align="center">
  <img
    src="https://res.cloudinary.com/dwjbed2xb/image/upload/v1778700690/architecture_ukigg6.png"
    alt="Softadastra architecture"
    width="760"
  />
</p>

## What is Softadastra ?

Softadastra is a reliability foundation for applications used in real-world conditions.
It is built for environments where the internet can be slow, unstable, expensive, or unavailable.
Instead of stopping when the connection fails, an application built with Softadastra can keep saving work locally and synchronize later.

## Who is it for ?

Softadastra is designed for organizations and teams that cannot afford to lose data because of poor connectivity.
It can be useful for:

- companies operating in unstable network environments
- public institutions and government systems
- schools and universities
- healthcare and field operations
- logistics and local business tools
- developers building offline-ready applications

## What problem does it solve ?

Many applications depend too much on a constant internet connection.
When the connection fails, users can lose time, lose work, or stop completely.
Softadastra helps applications stay usable during those moments by making local work possible first, then synchronizing later when the connection is available again.

## What this repository provides

This repository contains the main Softadastra runtime and command-line tool.
It provides the base system used to run, inspect, and control Softadastra locally.
For developers who want to integrate Softadastra into their own C++ applications, use the SDK:

[github.com/softadastra/sdk](https://github.com/softadastra/sdk)

## Install

Linux and macOS:

```bash
curl -fsSL https://softadastra.com/install.sh | bash
```

Windows PowerShell:

```powershell
irm https://softadastra.com/install.ps1 | iex
```

The installer can install the Softadastra command-line tool and the C++ SDK.

## Learn more

- Website: [softadastra.com](https://softadastra.com)
- Documentation: [docs.softadastra.com](https://docs.softadastra.com)
- SDK: [github.com/softadastra/sdk](https://github.com/softadastra/sdk)

## License

Licensed under the Apache License, Version 2.0. \
See the [LICENSE](LICENSE) file for details.
