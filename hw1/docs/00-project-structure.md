# 00 — Proposed Folder Structure

> **Status:** Proposed modular organization for a C implementation; filenames and module boundaries are **not** prescribed by the assignment. See [README](../README.md) for convention labels.

## Suggested project tree

```text
hw1/
├── Makefile                     # [REQUIRED] produces ./hw1 via make hw1
├── README.md                    # Local build/run instructions
├── include/
│   ├── config.h                 # Configuration declarations
│   ├── server.h                 # Listener/master lifecycle
│   ├── agent.h                  # Per-connection agent behavior
│   ├── shared.h                 # Shared-state layout and synchronization
│   ├── job.h                    # Job/connection state model
│   ├── scheduler.h              # Job selection policy
│   ├── allocator.h              # Processor feasibility and accounting
│   ├── protocol.h               # Text protocol and message formatting
│   └── report.h                 # REPORT generation
├── src/
│   ├── main.c
│   ├── config.c
│   ├── server.c
│   ├── agent.c
│   ├── shared.c
│   ├── job.c
│   ├── scheduler.c
│   ├── allocator.c
│   ├── protocol.c
│   └── report.c
├── docs/                        # These planning documents
│   ├── 00-project-structure.md
│   ├── 01-implementation-sequence.md
│   ├── 02-milestones.md
│   ├── 03-integration-tests.md
│   └── 04-final-architecture.md
└── tests/                       # [PROPOSED] your own fixtures/scenarios
    ├── configs/
    │   ├── single_node.txt
    │   ├── multi_node.txt
    │   └── contention.txt
    └── scenarios/
        ├── basic.md
        ├── concurrent.md
        └── timeout.md
```

Directories and filenames are illustrative. You **do not** need separate Unix processes for the scheduler, allocator, or nodes merely because they have separate modules.

## Module responsibility map

| Module | Owns / does | Should avoid |
| --- | --- | --- |
| `main` | Program entry; initialize config, shared resources, start master | Per-client protocol details |
| `config` | Parse `M N`, followed by `N` rows of `M` capacities from stdin | Mutating live job allocations |
| `server` | Choose `AF_UNIX` or `AF_INET`; socket, bind, listen, accept; fork agents; cleanup/reap children | Scheduling rules |
| `agent` | Per-client socket thread; timer/notification thread; connection lifecycle | Separate copies of global node capacity |
| `shared` | Shared mappings, slot management, process-shared mutex/condition variables | Client-specific parsing buffers |
| `job` | Job states and permitted transitions; connection-to-job association | Choosing the next job using policy |
| `scheduler` | Job eligibility and ranking; scheduler-trigger handling | Parsing socket commands |
| `allocator` | Test feasibility; choose nodes per processor type; reserve/release capacity | Decide which client has priority |
| `protocol` | Parse `SUBMIT`, `STOP`, `REPORT`, `QUIT`; format replies/notifications | Own shared resource state |
| `report` | Consistent snapshot; required formatting and sorting | Modify allocations |

## Important placement decisions

**[REQUIRED] Shared memory** holds job/scheduler state, node capacities, connection metadata and synchronization primitives (HW1 pp. 3–4). Ordinary process-local memory cannot be the authoritative copy of these fields.

**[PROPOSED] Agent-local memory** holds the accepted socket descriptor, receive buffer, parsing scratch space, and socket-write coordination. The accepted socket FD is owned by the agent connection, not an integer another agent can directly use as a socket descriptor.

**[PROPOSED] State representation** uses fixed-size shared arrays with slot IDs/indices, not raw pointers to unrelated `malloc` allocations. This follows the PDF's recommendation to use fixed regions/free slots.

**[PROPOSED] Logical separation** keeps job ranking and node assignment distinct. A schedule decision should not silently change client-protocol responsibilities.

## Build and submission constraints

- **[REQUIRED]** Language: C or C++; only standard libraries and pthread/real-time facilities allowed as stated in HW1.
- **[REQUIRED]** `make hw1` builds an executable called `hw1` in the project root (not under `build/`).
- **[REQUIRED]** Runtime syntax: `./hw1 conn`, with config read from standard input.
- **[REQUIRED]** Both supported connection formats need to be considered: `@/path/to/socket` for Unix-domain stream sockets, and `IPv4:port` for IPv4 stream sockets (HW1 p. 4).
- **[REQUIRED]** Official upload: `.tar.gz` containing a `hw1/` folder with its Makefile; check the instructor's command sequence on p. 5.
- **[PROPOSED]** Keep generated binaries, editor metadata and machine-specific socket paths out of the submitted archive.

## Directory design tips

- Keep exported interfaces small; don't expose internals of every module in every header.
- Decide which module is responsible for each shared-state mutation.
- Keep network I/O separate from critical sections that must hold process-shared locks.
- Allow `allocator` and `scheduler` to be reasoned about without a running socket server.
- Track invariants and source ambiguities in documentation instead of burying unverified assumptions in code.

**Source:** `ceng536-hw1.pdf`, §§2–6 (pp. 2–5). The folder/module split is a design recommendation, not a submission requirement.
