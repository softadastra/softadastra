# Host-to-Host

## Retained model

Host-to-Host is an optional direct connection between two independent Hosts:

```text
Host A <-> TLS 1.3 <-> Host B
```

The endpoint is configured explicitly by address and port. This is sufficient for the first LAN use cases and avoids a global discovery protocol. A Host remains useful when no peer is reachable.

## Identity and authorization

Each Host presents the existing persistent `HostIdentity::id()` value. The remote administration secret is never used as Host identity and is never sent as one.

The peer identity must be checked against the identity explicitly expected by the caller before any future capability exchange. Identity does not grant remote administration: the Phase 8 administration secret remains a separate credential and Host-to-Host does not expose administration commands.

TLS 1.3 protects the direct channel. The peer contract is limited to identity, reachability and non-sensitive infrastructure capabilities. It excludes software data, files, registrations and distributed commands.

## Discovery and unreliable networks

Automatic discovery is intentionally absent. Operators provide a LAN address, or later a known reachable address. mDNS is not required for the current direct model.

Connection loss has no retry loop, no persistence side effect and no effect on HostLoop, hosted software, storage or local control. A new explicit connection can be attempted after reachability returns.

## NAT and relays

Direct Internet connectivity requires a routable address or an explicit port forwarding rule. NAT-to-NAT connectivity is not claimed. A relay may become necessary for a demonstrated deployment, but none is implemented now. Any future relay must only forward encrypted Host-to-Host traffic and must not receive hosted software data.

## Regional use

Independent Boxes in separate shops or sites can retain local hosting during an Internet outage. When a configured direct path is available, they may exchange only the minimal Host infrastructure information above. This is not a cluster, coordinator, scheduler, replication system or distributed store.
