# Project Instructions for AI Agents

`AGENTS.md` is the authoritative operating manual for Courier 404. Read it after `bd prime`.

This repository uses Beads for durable task state and supports both single-agent and orchestrated multi-agent execution.

## Recovery

```bash
bd prime
bd ready --json
git status --short
git branch --show-current
git log --oneline -12
```

Then determine the explicit session role:
- lead/single-agent;
- orchestrator/integrator on `main`;
- worker on `agent/w1`, `agent/w2`, or `agent/w3`.

For orchestrated sessions also read:
- `docs/agent/multi-agent-orchestration.md`
- `docs/agent/night-run.md`

## Beads

Beads is the only persistent task tracker. Run `bd prime` for command details.

In orchestrated mode only the orchestrator mutates Beads state. Workers do not claim/close/reprioritize issues or edit dependencies.

## Git / UE lane

Workers commit only on their worker branch. The orchestrator integrates to `main` and owns all serialized UE build/cook/package/real-render verification.

Do not substitute Arch system clang for the UE bundled toolchain. Build commands live in `docs/architecture/build-commands.md`.
