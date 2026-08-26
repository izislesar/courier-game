# Autonomous Production Run Protocol

## Lead-agent objective

Advance the **current Beads production-readiness graph** dependency-first with truthful verification. Do not resurrect completed pre-production work and do not invent a parallel roadmap.

## Startup / recovery

```bash
bd prime
bd ready --json
git status --short
git log --oneline -12
```

Read `AGENTS.md`, `docs/product/production-readiness.md`, then the chosen issue and only relevant supporting docs.

If a compaction/model summary conflicts with repository state, recover from Beads/git before doing any work.

## Loop

1. Query ready work.
2. Prefer P0/P1, release blockers and dependency-unlocking work.
3. Claim one issue.
4. Read issue acceptance criteria and relevant docs/spec/plan.
5. For non-trivial changes, follow the appropriate Superpowers workflow.
6. Implement narrowly.
7. Run targeted tests first, then required broader build/integration checks.
8. Review spec/compliance and code quality.
9. Fix findings within issue scope.
10. Record newly discovered unrelated work in Beads.
11. Commit only when the active agent/user policy permits it.
12. Close/update the issue with exact verification evidence.
13. Re-query `bd ready --json`.
14. Continue.

Do not hold multiple unrelated claimed issues unless the harness explicitly coordinates independent workers.

## Visible runtime policy

Do not repeatedly launch visible Courier404 windows for automated profiling while the user is working.

Prefer:
- commandlets;
- `-nullrhi` for non-renderer checks;
- bounded offscreen/renderer boots where valid;
- logs/manifests/automated audits;
- one explicit human-playtest request when subjective evidence is required.

If exact frame/visual evidence cannot be captured non-disruptively, create/update the appropriate Beads gate and report the limitation rather than looping on increasingly invasive launch attempts.

## Discovery

For newly discovered required work, create a Beads issue with exact evidence and link it to the source issue using `discovered-from` where supported. Add blocking dependencies only when the relationship is actually blocking.

Do not implement unrelated discoveries inline.

## Long-run exit conditions

Stop normal work when:
- the active production epic/gate closes; or
- no ready work remains and remaining items are genuinely blocked/human-only.

Before ending:

```bash
bd ready --json
bd blocked --json || bd blocked
git status --short
```

Run the final applicable build/test gates and write/update a phase-appropriate report under `docs/reports/`.

The report should include:
- playable state;
- exact automated evidence;
- exact human-playtest requirements;
- performance evidence/limitations;
- open P0/P1 blockers;
- launch/build/package commands;
- no marketing language and no fabricated success.
