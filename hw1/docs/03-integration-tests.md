# 03 — Integration Test Scenarios

> **Purpose:** A manual test plan suitable for use from separate Cursor terminals. These are **author-created examples**, not official test cases from the assignment. The PDF specifies input, command and report formats but does **not** include a complete ready-to-run test suite.

## A. Before testing

**Required runtime contract** (`ceng536-hw1.pdf`, pp. 4–5):

- Launch: `./hw1 conn` (configuration is read from standard input).
- `conn` beginning with `@` indicates a Unix-domain socket **pathname**; otherwise use `ip:port` (IPv4).
- Agent sends its PID as a text connection ID on successful connection.
- Supported commands: `SUBMIT complete P1 ... Pm`, `STOP`, `REPORT`, `QUIT`.
- Submission reply: `OK`; start and deadline notifications have specified text.
- The basic time unit is **1 millisecond**.

These examples assume a Unix-like environment with `nc` supporting `-U` for Unix sockets. Some `nc` implementations vary by OS. Use **separate terminals** for simultaneous clients.

## B. Test fixture files (contents to create yourself)

### `tests/configs/single_node.txt`

```text
1 1
5
```

Interpretation: one processor type, one node with five processors of that type.

### `tests/configs/multi_node.txt`

```text
4 3
5 0 3 3
2 4 0 2
1 2 4 0
```

Interpretation: four processor types and three nodes with the listed capacity vectors.

### `tests/configs/contention.txt`

```text
1 2
5
2
```

Interpretation: one type and two nodes (five processors on Node 0; two on Node 1). A job needing three processors cannot split them between the nodes.

## C. Start the server and connect clients

### Unix-domain path

**Terminal 1** — from the HW1 project root:

```bash
make hw1
./hw1 @/tmp/hw1.sock < tests/configs/contention.txt
```

**Terminal 2**:

```bash
nc -U /tmp/hw1.sock
```

**Terminal 3** (a second client):

```bash
nc -U /tmp/hw1.sock
```

Each connection should first receive a **different agent PID** as plain text. The `@` is the homework's CLI prefix; the filesystem socket is `/tmp/hw1.sock`.

> If a previous server crashed, a stale Unix-domain pathname might remain. Confirm that no server owns the path before removing it.

### IPv4/TCP path

Restart the server with:

```bash
./hw1 127.0.0.1:8080 < tests/configs/contention.txt
```

Connect from another terminal:

```bash
nc 127.0.0.1 8080
```

**Expected:** Connection behavior and command protocol remain the same; only address setup differs.

## D. Manual scenario walkthroughs

### T01 — Connection and graceful quit

**Config:** `single_node.txt`

1. Start the server.
2. Connect one `nc` client.
3. Confirm the server sends an agent PID.
4. Enter `QUIT`.
5. Open a new client connection.

**Expected:** First agent exits and its connection metadata is removed. Master remains available and creates a new agent for the next connection. The PDF does not mandate a particular response string to `QUIT`.

### T02 — Feasible immediate job

**Config:** `multi_node.txt`

Send:

```text
SUBMIT 5000 2 0 1 0
```

**Expected:** `OK`, followed by an asynchronous assignment notification similar in format to:

```text
Job started at: <0 - 0 ->, completion in 5000
```

The sample node IDs here are only one *possible* valid allocation; an implementation may select another feasible node assignment. After the declared duration, expect:

```text
Active job is terminated by completion time
```

This job requests two Type 0 processors and one Type 2 processor. In the example configuration, Node 0 can supply both types.

### T03 — Contention and waiting

**Config:** `contention.txt`

Client A:

```text
SUBMIT 15000 5
```

Soon afterwards, Client B:

```text
SUBMIT 1000 3
```

**Expected:** While Client A holds all five Type 0 processors on Node 0, Client B cannot be assigned: Node 1 has only two, and a single requested type cannot span nodes. Client B remains waiting. When Client A stops or reaches its deadline, a scheduling event can make Client B runnable. Check both clients' notifications and the changing available counts.

> The 15-second duration is just a manual-test convenience. The instructor has not specified these input values.

### T04 — STOP releases processors

**Config:** `contention.txt`

1. Reproduce T03, with B waiting.
2. Send `STOP` in Client A's terminal before A expires.
3. Observe Client B and request `REPORT` from an independent connected client.

**Expected:** A's allocation is released exactly once, the waiting jobs are reconsidered, and B can be assigned if the scheduling policy selects it. An exact STOP acknowledgment string is **not** specified in the PDF.

### T05 — Infeasible request remains waiting

**Config:** `contention.txt`

Send:

```text
SUBMIT 20000 6
```

**Expected:** No node can supply six Type 0 processors. The job must not be falsely assigned by combining the nodes' capacities. It remains waiting unless canceled/disconnected; no negative counts appear in reports.

### T06 — Simultaneous submissions

**Config:** `single_node.txt`

Connect 3+ clients and submit resource requests in rapid succession so their combined demand exceeds five processors. Compare the final running allocations with the configured capacity.

**Expected:** No over-allocation, duplicate job slot, or lost connection entry. Scheduling order must follow the assignment rules, not merely socket arrival order when decisions are contested.

### T07 — Timeout versus STOP race

**Config:** `single_node.txt`

1. Submit a short job.
2. Send `STOP` very close to its completion deadline.
3. Repeat multiple times.

**Expected:** Regardless of which termination cause wins, allocated processors are released once and the connection remains in a coherent state. The PDF defines the timeout notification, but not an exact message for the race's losing event.

### T08 — Unexpected client disconnect

**Config:** `contention.txt`

1. Connect and start a job.
2. Close/terminate the `nc` process without sending `QUIT`.
3. Connect another client and inspect system status.

**Expected:** Disconnection has the same effect as QUIT: the agent exits; its submitted job, resources and connection entry are cleaned up.

### T09 — REPORT while state changes

**Config:** `contention.txt`

1. Keep one job running and another waiting.
2. Request `REPORT` from another client while a STOP or deadline expiry occurs.
3. Repeat.

**Expected:** A report corresponds to a consistent state, not partially updated counters and job assignments. The PDF requires node data, active jobs, waiting jobs, average wait time, and a waiting list.

### T10 — Ordering and output-format checks

**Config:** Any nontrivial fixture.

**Expected:** Node IDs follow input order; active jobs sort by **increasing remaining time**; waiting jobs sort by **decreasing waiting time**; unrequested processor types show `-` in assigned-node output. All timing values use milliseconds.

### T11 — Starvation / MAX rule

**Config:** A workload with one long-waiting job and intermittent feasible smaller jobs.

**Expected:** Once a job meets the assignment's `w >= MAX` condition, the specified top-priority/blocking rule applies. **[OPEN]** The PDF does not define a numeric `MAX`; confirm it before writing a test that expects a threshold at a particular millisecond value.

### T12 — No preemption

**Config:** `contention.txt`

Start a long running job, then submit another job with a better ordinary scheduling score or higher wait time.

**Expected:** The running job is never forcibly preempted before its declared completion time merely because another job becomes preferable. It can end by STOP, QUIT/disconnect or expiration.

## E. Test matrix

| ID | Area | Core invariant |
| --- | --- | --- |
| T01 | Connect/QUIT | Agent ends, master stays alive |
| T02 | SUBMIT/start/expiry | Correct protocol and timeout notification |
| T03 | Resource contention | No improper cross-node split for one type |
| T04 | STOP | Release and reschedule |
| T05 | Infeasible allocation | No invented capacity |
| T06 | Concurrent SUBMIT | No over-allocation / slot collisions |
| T07 | STOP/timeout race | Exactly-once resource release |
| T08 | Disconnect | Same cleanup effect as QUIT |
| T09 | Concurrent REPORT | Consistent snapshots |
| T10 | Formatting | Sorting, `-` markers, milliseconds |
| T11 | `MAX` fairness rule | No starvation-rule bypass |
| T12 | Non-preemption | Running allocation persists until termination |

## F. Protocol-format reference from the PDF

Submission syntax:

```text
SUBMIT complete P1 P2 ... Pm
```

Start notification example **from HW1 p. 4**:

```text
Job started at: <0 - 0 1>, completion in 100
```

Timeout notification **from HW1 p. 4**:

```text
Active job is terminated by completion time
```

REPORT headline/sections **from HW1 pp. 4–5**:

```text
%d nodes, %d jobs running, %d jobs waiting
Node Assignment
----------------
Active Jobs:
Conn.     Running    Remaining Assigned Nodes
------------------------------------
Waiting Jobs:
Conn.     Waiting   Requested Nodes
------------------------------------
average wait time: %.2f
waiting list:
```

The real output includes row data and the PDF's stated column formatting and ordering; the listing above is a **section checklist**, not a full expected-output transcript.

## G. Regression checklist

- [ ] Test both `@/path` and `ip:port` listener branches.
- [ ] Confirm PID connection IDs on each new client.
- [ ] Verify `SUBMIT`, `STOP`, `REPORT`, `QUIT` and disconnect.
- [ ] Repeat T07 and T09 under many attempts to expose races.
- [ ] Compare resource totals before and after all clients disconnect.
- [ ] Build from a clean project checkout with `make hw1`.
- [ ] Re-read `ceng536-hw1.pdf` for exact output and grading requirements before submission.

**Primary source:** `ceng536-hw1.pdf`, §§1, 2, 4, 5 (pp. 1–5); all test fixtures, commands, durations and expected illustrative allocations above are independently constructed for study.
