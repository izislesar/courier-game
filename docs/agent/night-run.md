# Autonomous Night Run Protocol

## Lead-agent objective

Drive the Beads graph to the pre-prod gate with truthful verification.

## Startup

```bash
bd prime
bd ready --json
git status --short
```

Read `AGENTS.md` and authoritative docs.

## Loop

1. Query ready work.
2. Prefer P0/P1 and dependency-unlocking work.
3. Claim one issue.
4. Read the issue and relevant plan/spec section.
5. If implementation is non-trivial, use a fresh implementation subagent.
6. Implement narrowly.
7. Run tests/build/smoke verification.
8. Run spec/compliance review.
9. Run code-quality review.
10. Fix findings.
11. Commit.
12. Close Beads issue with verification evidence.
13. Re-query ready work.
14. Continue.

Do not hold multiple unrelated claimed issues in one agent context unless the harness is explicitly coordinating parallel independent workers.

## Context-loss recovery

A fresh agent should recover with only:

```bash
cat AGENTS.md
bd prime
bd ready --json
bd show <chosen-id> --json
```

Then read the linked spec/plan/docs.

Do not rely on previous conversation memory.

## Discovery

When an implementation issue reveals additional required work:

```bash
bd create "Concise issue title" -t bug -p 0 --description="Exact evidence and required outcome" --json
bd dep add <new-id> <source-id> --type discovered-from
```

If it actually blocks another issue, add the appropriate blocking dependency as well.

Do not implement unrelated discoveries inline.

## Morning exit conditions

Stop normal feature work when:
- final pre-prod gate closes; or
- no ready work remains and all remaining work is genuinely blocked.

Before ending:
- run final available build/tests;
- `bd ready`;
- `bd blocked`;
- `git status --short`;
- write/update `docs/reports/preprod-status.md`;
- commit the report if repository policy permits.

The report must include:
- what is playable;
- what was verified;
- exact remaining blockers;
- P0/P1 known bugs;
- commands needed to launch/build;
- no marketing language.
