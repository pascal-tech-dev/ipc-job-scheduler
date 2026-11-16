# 02 — Milestone-by-Milestone Implementation Guide

> This guide expands the sequence in [`01-implementation-sequence.md`](01-implementation-sequence.md). The checklist is **proposed study/implementation planning**, not generated homework source.

## Milestone 1 — Configuration and data model

**Outcome:** A validated representation of the configured computing facility and the states needed for jobs and connections.

**Tasks**

- [ ] Read `M N` followed by exactly `N` rows, each containing `M` nonnegative processor counts.
- [ ] Distinguish **total** and **available** resources for every node/type pair.
- [ ] Decide how shared job and connection slots will be identified and reused within `MAXJOBS = 10000`.
- [ ] Model at least waiting/running job status, duration, connection/agent ownership, assigned node per requested type, and timing data.
- [ ] Define explicit invariants: available capacity never negative/exceeds configured capacity; no double assignment; one job per connection.
- [ ] Specify how timestamps and durations are interpreted in milliseconds.

**Verification**

- Load a 1-node, 1-type configuration and a multi-node, multi-type configuration.
- Reject malformed/negative/incomplete configurations predictably.
- Explain the difference between a node's **simulated processors** and a Unix process: there is no requirement to fork one process per node.

**Pitfall:** Mixing configured capacity and remaining capacity prevents accurate release/reporting.

## Milestone 2 — Master listener and agent processes

**Outcome:** Multiple independent clients connect over stream sockets and each is served by a separate agent process.

**Tasks**

- [ ] Select Unix domain (`@/path`) versus IPv4 (`ip:port`) from `conn`.
- [ ] Create/bind/listen on the appropriate stream socket.
- [ ] Let the master accept connections and fork a new agent per connection.
- [ ] Identify/close unused socket descriptors in both parent and child.
- [ ] Have each agent send its **own PID** as the connection ID immediately after connection.
- [ ] Decide how the parent reaps terminated children and how the listener cleans up on shutdown.
- [ ] Verify disconnection does not terminate the master.

**Verification**

- `nc -U /tmp/hw1.sock` connects to a Unix-domain server.
- `nc 127.0.0.1 8080` connects to an IPv4 server.
- Two simultaneous clients receive distinct agent PIDs.
- QUIT/disconnect ends an agent without leaving an occupied shared connection slot (once shared state is integrated).

**Pitfall:** Treating `accept()`'s return value as a connection ID. The assignment wants the agent PID, not the accepted file descriptor.

## Milestone 3 — Shared memory and process-shared synchronization

**Outcome:** All agents see a single authoritative scheduler/resource state and can update it safely.

**Tasks**

- [ ] Allocate shared state in the master **before `fork()`**.
- [ ] Consider anonymous, shared `mmap()` or `shmget()`; the PDF recommends `MAP_ANONYMOUS` and 4096-byte alignment for the mapping size.
- [ ] Place jobs, connections, node capacities, and synchronization objects in shared memory.
- [ ] Use free-slot bookkeeping or fixed arrays for shared records, not pointers to private `malloc` storage.
- [ ] Initialize mutex and condition-variable attributes with `PTHREAD_PROCESS_SHARED` where these objects coordinate different processes.
- [ ] Define a consistent lock scope/order and a wakeup protocol for agents whose jobs change state.
- [ ] Ensure cleanup does not destroy a shared synchronization object while other agents are using it.

**Verification**

- Changes made by Agent A are visible to Agent B.
- Two agents concurrently updating the same shared record do not lose modifications.
- Waiting/wakeup paths are tested for missed notifications and spurious wakeups.

**Pitfall:** An ordinary process-private mutex does not protect shared state across agents, even if every agent happens to declare one.

## Milestone 4 — Processor allocation

**Outcome:** The allocator correctly tests feasibility, records node choices, reserves and releases resources.

**Tasks**

- [ ] For every *requested processor type*, find a **single** node with enough currently available processors of that type.
- [ ] Permit different types from different nodes.
- [ ] Record the selected node for each type; represent unrequested types as unassigned (`-` in client-facing output).
- [ ] Make reservation consistent: either all requested types reserve successfully or none do.
- [ ] Release exactly those resources reserved for the job, once.
- [ ] Pick a deterministic rule when several nodes have sufficient capacity (**[PROPOSED]**, not defined by the PDF).

**Worked reasoning**

```text
Node 0: <5, 0, 3, 3>
Node 1: <5, 0, 3, 3>
Request: Type 0 = 6  -> NOT feasible (neither node has 6)
Request: Type 0 = 2, Type 2 = 2 -> feasible (may use different nodes)
```

**Verification**

- An infeasible request makes no changes to capacities.
- Successful reservation never produces a negative count.
- Reserve → release returns every node count to its original value.
- Two concurrent agents cannot reserve the same processors beyond capacity.

**Pitfall:** Summing resources across nodes for one processor type; this violates the explicit HW1 example.

## Milestone 5 — Scheduling policy

**Outcome:** Given waiting jobs and a consistent resource snapshot, the scheduler selects a job according to the stated rules.

**Tasks**

- [ ] Determine which jobs have waited at least `MAX` (when `MAX` is defined/clarified).
- [ ] Apply the **starvation rule** first: select among the maximum-wait jobs with `w >= MAX`; if the chosen job cannot be allocated, do not assign another job in that scheduling attempt.
- [ ] Otherwise exclude infeasible jobs.
- [ ] Compare wait time (larger is better) and processor budget (smaller is better).
- [ ] If no job is simultaneously best on both dimensions, calculate each job's domination count over other eligible jobs.
- [ ] Apply the PDF's literal final criterion: among best domination-count candidates, choose the **minimum** `wait − budget`.
- [ ] Confirm that a running job is **not preempted** within its declared completion duration.
- [ ] Make scheduling happen on both new-job arrival and termination of an existing job.

**Verification**

- Manually construct tables containing each job's wait, budget, feasibility, and domination count.
- Verify that the same input yields the same selection repeatedly (subject to explicitly chosen tie handling).
- Check the `MAX` case separately from ordinary feasible-job ranking.

**[OPEN] Questions to clarify with instructor/forum**

- The PDF names `MAX` but does not give a numeric value.
- `wait − budget` with **minimum** selection may feel counterintuitive, but it is the actual printed rule; preserve it until clarified.
- Remaining ties and selection among multiple equally old `w >= MAX` jobs are not fully specified.

**Pitfall:** Choosing a common scheduler such as FIFO/SJF in place of the assignment's decision procedure.

## Milestone 6 — Job lifecycle, two threads, notifications

**Outcome:** End-to-end command handling, waiting, running, cancellation, expiry, and asynchronous client messages.

**Tasks**

- [ ] Give each agent a **socket-input thread** that may block on reads.
- [ ] Give each agent a **time/notification thread** that can observe assignment changes and deadlines without waiting for socket input.
- [ ] For SUBMIT, register a job and send the required immediate `OK`, then make the job eligible for scheduling.
- [ ] On assignment, record node choices, transition to running, and send the required job-start notification from the owning agent.
- [ ] On declared duration expiry, terminate the simulated job, release processors, notify its client and trigger scheduling again.
- [ ] For STOP, cancel the current submitted job whether it is waiting or running, releasing resources if allocated.
- [ ] For QUIT and unexpected socket close, remove job and connection state and terminate the agent.
- [ ] Prevent double termination/release if STOP, expiry, or disconnect races occur.
- [ ] Serialize writes from the agent's two threads to its client socket; avoid blocking socket I/O while holding global shared-state locks.

**Verification**

- The client can receive an asynchronous job-start message without first issuing another command.
- Expiration sends `Active job is terminated by completion time`.
- Two simultaneous termination causes lead to only one resource release.
- After one job ends/cancels, a previously waiting job can become assigned.

**Pitfall:** Assuming a notification thread in Agent A can directly write to Agent B's client socket using Agent B's file-descriptor number. Each agent should notify its own client.

## Milestone 7 — REPORT, integration, packaging

**Outcome:** Fully observable and reproducible behavior consistent with HW1 output/packaging rules.

**Tasks**

- [ ] Capture a consistent view of node availability, running jobs and waiting jobs.
- [ ] Format the `REPORT` headings and columns using the specified template.
- [ ] Sort nodes by original ID, running jobs by increasing remaining time, and waiting jobs by decreasing wait.
- [ ] Show `-` for unrequested processor types in assigned-node output.
- [ ] Keep all reported units in milliseconds and include the specified average wait time.
- [ ] Validate concurrency and cleanup scenarios in [`03-integration-tests.md`](03-integration-tests.md).
- [ ] Ensure `make hw1` creates `./hw1` and test the official archive directory layout.

**Verification**

- `REPORT` remains consistent while concurrent submissions/stops occur.
- No negative available counts or more than one job per connection.
- Exact required notifications are preserved.
- The program can be built and launched from the extracted `hw1/` directory.

**Pitfall:** Treating `REPORT` as a debugging printout instead of a protocol requirement with stated sorting and formatting.

## End-to-end invariants to keep visible

1. **Each connection owns at most one job at a time.**
2. **Each requested type is supplied by exactly one node** (though separate types may use separate nodes).
3. **Never allocate more than the available processors** of any node/type.
4. **Every reservation is eventually released once**, through timeout, STOP, QUIT, or disconnect.
5. **No shared scheduler mutation occurs without adequate cross-process synchronization.**
6. **Running jobs are not preempted** before their declared completion limit.
7. **Only the owning agent communicates with its client** in the proposed design.

**Primary source:** `ceng536-hw1.pdf`, §§1–6 (pp. 1–5). Acceptance gates and module responsibilities are recommendations.
