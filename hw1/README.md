# CENG536 HW1 — Architecture & Implementation Planning Pack

This package is intended to be copied into a **Cursor development workspace** as reference documentation. It contains **planning material, not submission-ready source code**.

## Start here

| Document | Purpose |
| --- | --- |
| [`docs/00-project-structure.md`](docs/00-project-structure.md) | Suggested module layout, responsibilities, build constraints |
| [`docs/01-implementation-sequence.md`](docs/01-implementation-sequence.md) | Dependency-ordered implementation roadmap and completion gates |
| [`docs/02-milestones.md`](docs/02-milestones.md) | Detailed tasks, verification criteria, and design pitfalls per milestone |
| [`docs/03-integration-tests.md`](docs/03-integration-tests.md) | Test configurations, `nc` workflows, scenario matrix and expected behavior |
| [`docs/04-final-architecture.md`](docs/04-final-architecture.md) | Combined architecture, Mermaid diagrams, event flows and shared-state rules |

## Reference and scope

Primary source: **`ceng536-hw1.pdf` — METU, CEng 536 Advanced Unix, Fall 2026, HW1 (5 pages; due November 22, 2026)**. The documents also preserve the *two user-drawn diagrams* and the *planning outline discussed in this conversation*. Source requirements and recommended engineering choices are distinguished explicitly.

**Course-integrity note:** Page 5 prohibits use of generative AI in resulting source code beyond simple completion and syntax correction. These files are conceptual study/planning notes. Write and validate your own implementation, and follow the instructor's rules.

**Abbreviations and conventions:**

- **[REQUIRED]** — explicit or strongly implied by the assignment.
- **[PROPOSED]** — an engineering design suggestion, not mandated.
- **[OPEN]** — ambiguous or undefined in the PDF; verify with the instructor/forum if it matters.
- `M` — number of processor types; `N` — number of nodes.
- `MAXJOBS = 10000` — maximum jobs and connections.
- All durations and report times use **milliseconds**.

## Using the pack in Cursor

1. Put the `docs/` directory under your existing `hw1/` project (or keep it alongside the project while planning).
2. Review [`04-final-architecture.md`](docs/04-final-architecture.md) to understand process boundaries.
3. Work through [`01-implementation-sequence.md`](docs/01-implementation-sequence.md), consulting [`02-milestones.md`](docs/02-milestones.md) at each stage.
4. Validate each completed stage using [`03-integration-tests.md`](docs/03-integration-tests.md).
5. Treat the source PDF as authoritative wherever these planning notes and the assignment differ.

**Do not submit this planning ZIP as your homework deliverable.** The official submission expects a `hw1/` directory inside a `.tar.gz` and a `Makefile` such that `make hw1` creates `hw1` in the project directory.
