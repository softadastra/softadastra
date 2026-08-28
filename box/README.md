# Softadastra Box, Initial Profile

A Box is a dedicated Linux Host. It does not introduce a different architecture.

## Hardware Requirements

Minimum: x86_64 CPU with 2 cores, 4 GB RAM, 64 GB SSD, Gigabit Ethernet, stable power supply, and passive cooling or cooling suitable for continuous operation.

Recommended: x86_64 CPU with 4 cores, 8 GB RAM, 128 GB or larger NVMe SSD, Gigabit Ethernet, Wi-Fi 5 or newer, two USB ports, a high-quality external power supply, properly sized passive or active cooling, and firmware that supports automatic restart after power is restored.

CPU, RAM, and storage are Host capacities, not manufacturer-specific requirements. The first prototype targets an x86_64 mini PC compatible with Debian or Ubuntu LTS, with replaceable SSD storage and Ethernet. No specific hardware model has been physically validated by this repository.

## System Image

Use a minimal Debian 12 or Ubuntu 24.04 LTS x86_64 installation with an up-to-date system, `systemd`, local SSD storage, and networking managed by the distribution.

Then install the built binary and run `box/install.sh` as root. The script creates the Host user and directories, installs the systemd unit, enables it, and starts the Host. It downloads nothing and does not depend on Internet access after installation.

The script refuses to overwrite an existing binary or systemd unit. An update is therefore a separate, explicit operation.

## First Boot

1. Verify in the firmware that automatic restart after power loss is enabled.
2. Install a minimal Linux system on the SSD and apply the initial updates.
3. Copy the `softadastra` binary to the Box and run `sudo box/install.sh ./softadastra`.
4. Verify `systemctl status softadastra`, then run `softadastra access` and `softadastra connectivity`.
5. Register third-party software and verify its start, stop, and restoration behavior.

## Hardware Validation

- Measure continuous operation, temperature, and power consumption on the actual hardware.
- Verify Ethernet and Wi-Fi loss and recovery without losing local control.
- Test power loss and restoration with firmware restart enabled.
- Perform multiple reboots and verify that registrations are restored.
- Test real long-running processes, local servers, command-line arguments, file writes, and crashes.

The physical prototype remains unbuilt until an actual machine has been assembled, installed, and tested.
