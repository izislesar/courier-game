# Autonomous Multi-Agent Production Run Protocol

## Objective

Advance the current Courier 404 production-readiness graph overnight using one orchestrator/integrator and up to three isolated workers without corrupting task state, git history, or the UE verification lane.

## Roles

- `main`: orchestrator/integrator only.
- `agent/w1`: worker 1.
- `agent/w2`: worker 2.
- `agent/w3`: worker 3.
- Beads mutation: orchestrator only.
- UE build/cook/package/real-render verification: orchestrator only and serialized.

## Orchestrator startup

```bash
bd prime
bd ready --json
bd blocked --json || bd blocked
git status --short
git log --oneline -12
./scripts/agent-bus.sh init
./scripts/agent-bus.sh status
./scripts/agent-bus.sh results
```

Read `AGENTS.md`, `docs/agent/multi-agent-orchestration.md`, and `docs/product/production-readiness.md`.

## Scheduling loop

1. Reconcile any existing worker result before issuing new work.
2. Query `bd ready --json`.
3. Prefer P0/P1 dependency-unlocking leaves.
4. Select at most one task per idle worker.
5. Reject parallel combinations with likely file/subsystem/config/content overlap.
6. Claim/update Beads from the orchestrator only if the chosen issue should be marked active.
7. Assign through `scripts/agent-bus.sh assign` with explicit scope, acceptance, forbidden overlap, and worker-safe verification.
8. Wait for results while other workers continue independently.
9. Inspect completed worker branch before integration.
10. Integrate with ancestry-preserving merge from `main`.
11. Run required UE verification serially.
12. If evidence passes, update/close Beads; otherwise record exact failure and keep/reopen scope truthfully.
13. Acknowledge the worker.
14. Fast-forward that clean worker branch to current `main` before its next assignment.
15. Re-query ready work and continue.

## Backlog feed rule

If fewer than three safe independent ready leaves remain, the orchestrator may perform a bounded decomposition pass on the next highest-priority broad issue.

New leaves must:
- correspond to real production acceptance work;
- be small enough for one worker cycle;
- have objective acceptance criteria where possible;
- declare dependencies accurately;
- avoid duplicate scope;
- avoid external assets/human-only work unless explicitly represented as a gate.

Do not manufacture filler issues merely to keep workers occupied.

## Worker loop

A worker repeatedly:
1. waits on its bus task;
2. validates branch/worktree identity;
3. reads only relevant context;
4. implements exactly assigned scope;
5. runs only worker-safe narrow checks;
6. reviews its diff;
7. commits locally;
8. reports commit + evidence through the bus;
9. waits for the next assignment.

Workers never close Beads issues and never run the serialized UE lane.

## Integration protocol

From `main`, for a completed worker such as w1:

```bash
git status --short
git diff main...agent/w1 --stat
git diff main...agent/w1
```

If scope is clean and non-conflicting:

```bash
git merge --no-ff --no-edit agent/w1
```

Then run issue-required verification. If integration fails, abort/reconcile truthfully; do not hide conflicts.

After successful integration + Beads update + bus ack, and only when the worker worktree is clean:

```bash
git -C ../courier-w1 merge --ff-only main
```

This keeps reusable worker branches current without cherry-pick divergence.

## Visible runtime policy

Do not spam visible Courier404 windows. Real-render checks must be bounded and owned by the orchestrator. Human subjective gates are requested explicitly rather than repeatedly approximated by automation.

## Failure handling

A worker failure is not an issue failure by itself. The orchestrator reads the reason and decides whether to:
- clarify/reassign the same issue;
- split the issue;
- create a blocker/follow-up;
- defer genuinely human/asset-dependent work.

No worker silently changes scope to work around a blocker.

## Night-run exit

Stop when:
- no safe ready work remains;
- remaining work is genuinely human/asset/environment blocked;
- integration/verification infrastructure is broken;
- a material product/architecture decision requires the user.

Before exit:

```bash
./scripts/agent-bus.sh status
./scripts/agent-bus.sh results
bd ready --json
bd blocked --json || bd blocked
git status --short
```

Write/update a concise report only if it adds durable evidence. Raw transient logs remain local unless intentionally promoted as evidence.
