# Host-to-Host

## Retained model

Host-to-Host is an optional direct connection between two independent Hosts:

```text
Host A <-> TLS 1.3 <-> Host B
```

The endpoint is configured explicitly by address and port. This is sufficient for the first LAN use cases and avoids a global discovery protocol. A Host remains useful when no peer is reachable.

## Identity and authorization

Each Host creates a self-signed Ed25519 TLS certificate from its persistent
`HostIdentity` private key. `HostIdentity::id()` is the SHA-256 fingerprint of
that certificate public key. The remote administration secret is never used as
Host identity and is never sent as one.

The TLS client extracts the certificate public key, hashes it with SHA-256 and
compares it to the explicitly configured HostId before sending a protocol
request. A HostId declared by the protocol is only cross-checked against that
certificate-derived value; it is never trusted on its own. Identity does not
grant remote administration: the Phase 8 administration secret remains a
separate credential and Host-to-Host does not expose administration commands.

TLS 1.3 protects the direct channel. Its one-request protocol accepts `identity`,
`ping` and `infrastructure`; the latter returns only non-sensitive infrastructure
text. It excludes software data, files, registrations and distributed commands.

## Discovery and unreliable networks

Automatic discovery is intentionally absent. Operators provide a LAN address, or later a known reachable address. mDNS is not required for the current direct model.

Connection loss has no retry loop, no persistence side effect and no effect on HostLoop, hosted software, storage or local control. A new explicit connection can be attempted after reachability returns.

## NAT and relays

Direct Internet connectivity requires a routable address or an explicit port forwarding rule. NAT-to-NAT connectivity is not claimed. A relay may become necessary for a demonstrated deployment, but none is implemented now. Any future relay must only forward encrypted Host-to-Host traffic and must not receive hosted software data.

## Regional use

Independent Boxes in separate shops or sites can retain local hosting during an Internet outage. When a configured direct path is available, they may exchange only the minimal Host infrastructure information above. This is not a cluster, coordinator, scheduler, replication system or distributed store.
