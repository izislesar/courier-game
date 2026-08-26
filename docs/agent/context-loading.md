# Agent Context Loading and Recovery Contract

## Goal

Any new model/session should recover correct project state without the previous chat transcript and without reading the entire repository.

## Universal recovery

After `/compact`, model/provider switch, new session, interrupted run, or stale-summary suspicion:

```bash
bd prime
bd ready --json
git status --short
git branch --show-current
git log --oneline -12
```

Then read `AGENTS.md` and determine the explicit role.

## Role-aware recovery

### Lead / single-agent

Read the chosen ready issue and relevant L1 docs. Normal one-issue workflow applies.

### Orchestrator / integrator

Also run:

```bash
./scripts/agent-bus.sh status
./scripts/agent-bus.sh results
bd blocked --json || bd blocked
```

Read `docs/agent/multi-agent-orchestration.md`. Recover which workers are `idle`, `assigned`, `working`, `done`, or `failed` before scheduling anything.

Never trust a stale chat statement about worker state over the bus + git branches.

### Worker

Confirm the expected branch:

```bash
git branch --show-current
./scripts/agent-bus.sh status
```

A worker does **not** choose work from `bd ready`. It waits for its assignment using the bus and reads only issue-relevant context.

## Context precedence

Task state:
1. Beads live state;
2. current git branches/worktrees;
3. agent bus for ephemeral assignment/result state;
4. issue-linked authoritative docs/specs;
5. latest verification evidence;
6. historical reports/plans;
7. chat/compaction summary.

Product/architecture decisions:
1. current user instruction;
2. `AGENTS.md`;
3. current product/architecture contracts;
4. Beads acceptance criteria;
5. issue-linked implementation plan;
6. historical docs.

The bus never overrides Beads task truth.

## Layered reading

L0:
- `AGENTS.md`
- recovery commands
- explicit role contract
- assigned issue/task
- `docs/product/production-readiness.md`

L1 only when relevant:
- gameplay -> `docs/product/gameplay.md`
- presentation -> `docs/product/art-direction.md`
- renderer/performance -> `docs/architecture/performance-budget.md`
- city -> `docs/architecture/city-simulation.md`
- system design -> `docs/architecture/ue5-architecture.md`
- verification/release -> `docs/architecture/testing-and-verification.md`

L2 evidence/history:
- `docs/reports/`
- `docs/archive/`

## Anti-staleness

- Archived/bootstrap task graphs are never current task state.
- Completed issues do not reactivate because old reports mention them.
- Reports are point-in-time evidence.
- Workers never infer a new assignment from an old prompt/result file after the orchestrator has acknowledged it.
- Reused worker branches must be resynchronized to main between assignments.
- Never create another persistent task tracker beside Beads.

## Durable handoff

Orchestrator Beads notes should preserve:
- what changed;
- worker commit/branch where relevant;
- integration result;
- commands run;
- pass/fail;
- human-only acceptance still required;
- exact blocker;
- discovered follow-up IDs.
