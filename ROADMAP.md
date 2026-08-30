# Softadastra Roadmap

Softadastra is being built in layers.

The order matters. Each stage should make the Host more useful without forcing hosted software to adopt a new application architecture.

The roadmap describes product capabilities, not individual implementation tasks.

## 0.1 - Host foundation

Build a Host that can run existing software reliably on Linux and Windows.

### Host

- [x] Start and stop the Softadastra Host
- [x] Report Host status and information
- [x] Persist Host operational state
- [x] Restore supervised software after Host restart

### Software lifecycle

- [x] Initialize a project with `softadastra.toml`
- [x] Register software
- [x] Run software from the current project
- [x] Start registered software
- [x] Stop software
- [x] Restart software
- [x] Remove software from the Host
- [x] List registered software
- [x] Report software status and information
- [x] Capture software logs
- [x] Clear logs
- [x] Follow logs
- [x] Report useful launch failures

### Access

- [x] Declare an application access point
- [x] Support multiple access points
- [x] Preserve compatibility with the original single-access format
- [x] Report how hosted software can be reached

### Local control

- [x] Local Host control on Linux
- [x] Local Host control on Windows
- [x] Unix-local transport on Linux
- [x] Named Pipe transport on Windows
- [x] Stable local endpoint identity between processes

### User interface

- [x] Command-line interface
- [x] Local Web interface
- [x] Application inventory
- [x] Application lifecycle controls
- [x] Project configuration editing
- [x] Log access

### Reliability

- [x] Unit tests
- [x] Integration tests
- [x] CLI end-to-end tests
- [x] Linux GCC CI
- [x] Linux Clang CI
- [x] Windows MSVC CI
- [x] Sanitizer validation
- [x] Static analysis
- [x] Strict warnings for Softadastra code

### Distribution

- [ ] Validate the complete Linux package
- [ ] Validate the complete Windows package
- [ ] Validate installation from a clean machine
- [ ] Make first-time installation extremely simple
- [ ] Publish the first stable `0.1.x` release

## 0.2 - Local reachability

Make software running on a Host easy to reach from the local machine and local network.

The application should not need to know how the Host provides that reachability.

### Existing networks

- [ ] Reliable discovery of usable local interfaces
- [ ] Stable local application addresses
- [ ] Clear access information for users on the same network
- [ ] Reliable behavior when network interfaces change

### Managed local networking

Primarily for Linux Hosts and dedicated machines.

- [ ] Managed local network lifecycle
- [ ] Local DNS
- [ ] Local gateway
- [ ] Dedicated Host network
- [ ] Safe coexistence with an existing network
- [ ] Recovery after network interruption

A personal computer should not lose its normal connectivity simply because Softadastra is installed.

## 0.3 - Softadastra Box

Turn a dedicated machine into a Host that can operate continuously with minimal administration.

```text
dedicated machine
       +
   Softadastra
       ↓
      Box
```

The Box remains a Host. Applications should not need a separate Box-specific architecture.

### Provisioning

- [ ] Define the first reference hardware profile
- [ ] Provision a compatible machine automatically
- [ ] Configure Host startup
- [ ] Configure local networking
- [ ] Recover after power loss
- [ ] Restore hosted software automatically

### System image

- [ ] Define the minimal Linux system
- [ ] Build a reproducible installation image
- [ ] Make installation possible from removable media
- [ ] Reduce installation to a small number of user decisions

### Hardware validation

- [ ] Test real mini-PC hardware
- [ ] Record hardware-specific limitations
- [ ] Define certification requirements
- [ ] Publish the first list of validated machines

A machine must not be called certified until it has been physically tested.

## 0.4 - Secure remote reachability

Allow an owner to reach a Host from outside the local network without making Internet connectivity necessary for local operation.

```text
Software
   ↓
Host
   ├── local access
   └── secure remote access
```

### Identity

- [ ] Stable Host identity
- [ ] Host credentials
- [ ] Identity verification
- [ ] Credential rotation and recovery

### Transport

- [ ] Authenticated encrypted communication
- [ ] Secure outbound remote connection
- [ ] Remote application access
- [ ] Remote Host management
- [ ] Clear behavior when Internet connectivity disappears

Remote access must not be presented as safe for public Internet use until authentication, encryption, authorization, and recovery have been validated together.

## 0.5 - Multiple executions

Allow one Software entry to contain more than one supervised execution when real applications require it.

The model should remain generic.

For example:

```toml
[[execution]]
name = "server"
command = "vix run"

[[execution]]
name = "web"
directory = "frontend"
command = "npm run preview"
```

Softadastra should not introduce semantic categories such as `frontend`, `backend`, `database`, or `worker`.

They are executions.

### Capabilities

- [ ] Multiple executions per Software
- [ ] Independent execution lifecycle
- [ ] Working directory per execution
- [ ] Logs per execution
- [ ] Application-level lifecycle
- [ ] Failure propagation rules
- [ ] Startup and shutdown ordering when required

This stage should only begin when real applications demonstrate that the single-command model is no longer sufficient.

## 0.6 - Resource control

Allow Hosts and hosted software to express and enforce resource limits without introducing infrastructure-specific application configuration.

The intended concepts are small:

```text
ResourceBudget
ResourcePolicy
```

### Host resources

- [ ] CPU capacity
- [ ] Memory capacity
- [ ] Process capacity
- [ ] Host-wide resource accounting

### Software resources

- [ ] Per-application budgets
- [ ] Per-execution budgets where necessary
- [ ] Resource usage reporting
- [ ] Resource enforcement

### Policies

- [ ] Balanced policy
- [ ] Dedicated policy
- [ ] Custom policy

Platform-specific enforcement may use mechanisms such as Linux cgroups or Windows Job Objects, but those mechanisms should remain behind the same Softadastra model.

## 0.7 - Host-to-Host

Allow multiple Hosts to communicate and cooperate without turning the individual Host into a different product.

Host-to-Host should only be expanded when concrete product needs require it.

Possible capabilities include:

- [ ] Host discovery
- [ ] Trusted Host relationships
- [ ] Authenticated Host communication
- [ ] Software reachability between Hosts
- [ ] Coordination between Hosts
- [ ] Operation across intermittently connected networks

The architecture for this work is documented separately in `docs/host-to-host.md`.

## Later

Some ideas may become useful after the previous foundations are proven:

- fleet management
- automatic software distribution
- Host migration
- coordinated recovery
- resource placement across several Hosts
- larger Softadastra networks

These are directions, not commitments.

They should not be implemented merely because they are technically possible.

## Development rule

The roadmap should grow from real requirements.

When a new problem appears, the first question is not:

> Which new subsystem should Softadastra add?

The first question is:

> Can the problem be represented using the concepts that already exist?

Softadastra should remain a small set of concepts capable of supporting increasingly useful systems.
