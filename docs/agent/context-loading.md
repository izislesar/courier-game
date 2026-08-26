# Agent Context Loading and Recovery Contract

## Goal

A new model should recover the current project state without needing the previous chat transcript.

## Recovery invariant

After any of the following:
- `/compact`;
- provider/model switch;
- new agent session;
- interrupted autonomous run;
- stale summary suspicion;

run:

```bash
bd prime
bd ready --json
git status --short
git log --oneline -12
```

Then select a ready issue and run:

```bash
bd show <issue-id> --json
```

If the compacted/chat summary disagrees with Beads/git, **Beads/git wins**.

## Context precedence

For task state:
1. Beads live state;
2. current git working tree/history;
3. issue-linked authoritative docs/specs;
4. latest verification evidence;
5. historical reports/plans;
6. chat/compaction summary.

For product/architecture decisions:
1. current user instruction;
2. `AGENTS.md`;
3. current product/architecture contracts;
4. Beads issue acceptance criteria;
5. issue-linked implementation plans;
6. historical docs.

## Anti-staleness rules

- Completed epics do not become active because an old report mentions them.
- Archived bootstrap task graphs are never a task source.
- Reports describe evidence at a point in time; they do not own future work.
- `CONTEXT_VERSION` identifies the instruction-pack generation, not current task state.
- Never create `CURRENT_TASKS.md`, `TODO.md`, `roadmap-checklist.md`, or another persistent tracker competing with Beads.

## Efficient reading

Do not pre-load every product and architecture document for every task.

Load issue-relevant context only. This improves cache stability and reduces semantic pollution after compaction.

Examples:
- mouse feel: gameplay + testing; no need to load relationship/police implementation history;
- material pass: art direction + performance + production readiness;
- city density: city simulation + performance + art direction;
- persistence fix: UE architecture + testing + gameplay state contract;
- release gate: production readiness + testing + performance + latest reports.

## Handoff quality

A durable issue close/update should contain enough evidence that another model can continue without the chat:
- what changed;
- files/systems affected;
- commands run;
- pass/fail;
- exact blocker if incomplete;
- discovered follow-up issue IDs.
