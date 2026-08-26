# Multi-Agent Orchestration Contract

## Architecture

Courier 404 uses one durable task graph and four git execution lanes:

```text
Beads (task truth)
      |
main orchestrator/integrator
      |
shared git-common-dir agent bus
   /      |      \
w1       w2       w3
agent/    agent/   agent/
w1        w2       w3
```

The bus is ephemeral coordination state. Beads is durable task state.

## Why workers are separate worktrees

Parallel agents need isolated working directories and branches. They must not concurrently edit one shared working tree.

Expected layout:

```text
~/Projects/courier-game   main
~/Projects/courier-w1     agent/w1
~/Projects/courier-w2     agent/w2
~/Projects/courier-w3     agent/w3
```

All worktrees share one git common directory, so `scripts/agent-bus.sh` stores coordination under that common directory and every worktree sees the same bus state.

## Authority matrix

| Action | Orchestrator | Worker |
|---|---:|---:|
| Read Beads | yes | optional/read-only |
| Claim/update/close Beads | yes | no |
| Edit implementation | normally no | yes, assigned scope only |
| Commit worker implementation | no | yes, own branch |
| Merge to main | yes | no |
| UE build/test/package | yes | no |
| Real-render/profile | yes | no |
| Human acceptance claim | no; request human | no |
| Assign next work | yes | no |

## Assignment quality

Every assignment should contain:
- issue ID;
- concrete scope;
- acceptance criteria;
- relevant context docs/files;
- explicit forbidden overlap;
- worker-safe verification;
- commit/report requirement;
- stop condition.

A worker never gets vague prompts such as “improve the city” or “polish everything”.

## Overlap control

Before parallel assignment the orchestrator compares likely ownership/file footprints.

Do not parallelize tasks that both likely touch:
- `Courier404GameMode.*`;
- the same player/vehicle class;
- the same map or generated asset;
- `DefaultEngine.ini` / the same config group;
- package scripts or release gates;
- one shared test file;
- the same subsystem contract.

If uncertain, serialize the tasks.

## Bus lifecycle

Typical state transitions:

```text
idle
 -> assigned:<issue>
 -> working:<issue>
 -> done:<issue>   or failed:<issue>
 -> idle (after orchestrator ack)
```

The orchestrator must process and acknowledge a result before assigning the next task to that worker.

## Reusable worker branch lifecycle

Worker branches persist across tasks.

Correct cycle:
1. worker starts from current main;
2. worker commits assigned task;
3. orchestrator merges worker branch into main with `--no-ff`;
4. orchestrator verifies and updates Beads;
5. orchestrator acknowledges worker;
6. clean worker worktree fast-forwards to main;
7. worker receives next task.

This preserves ancestry and prevents stale-base accumulation.

## Why routine cherry-pick is discouraged

Cherry-picking worker commits creates different commit IDs on main. If the same worker branch is then reused, it may no longer fast-forward cleanly and can repeatedly carry already-integrated commits.

Use merge integration for persistent workers. Cherry-pick is reserved for exceptional/manual recovery, followed by explicit branch reconciliation.

## Serialized UE lane

The UE toolchain is heavyweight and project/engine artifacts are not treated as safely parallel across workers.

Workers therefore do not invoke UE project builds, automation, cook/package, or real renderer. The orchestrator validates integrated main once per completed task/batch as required.

This improves wall-clock parallelism in code authoring while avoiding resource contention and evidence races.

## Human gates

Subjective visual/feel acceptance stays outside worker/orchestrator automation. The orchestrator records the exact human gate and continues independent work when possible.

## Crash/recovery

After a crashed session:
- bus status tells ephemeral worker lifecycle;
- worker branch commits tell what code exists;
- Beads tells durable task state;
- git status tells uncommitted residue.

Never infer recovery state solely from the previous model's prose.
