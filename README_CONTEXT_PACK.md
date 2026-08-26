# Courier 404 — Agent Context Pack v2

This archive is an overlay for a freshly initialized Courier 404 repository.

## Purpose

The repository context is designed for one autonomous overnight implementation run that should end with a **pre-production vertical slice candidate**, not with an unconstrained attempt to build the full game.

The target slice must prove the complete fantasy:

1. live in a small believable urban space;
2. accept an ordinary delivery;
3. physically pick up cargo;
4. drive it across the district;
5. deliver it and get paid;
6. manage food, sleep, fuel and basic expenses;
7. interact with the girlfriend relationship loop;
8. receive and optionally accept an anonymous high-risk contract;
9. survive at least one police encounter;
10. survive or fail at least one hostile-NPC encounter;
11. be able to be injured, arrested, killed or starve after prolonged neglect;
12. save, quit, reload and continue.

The full product vision is deliberately larger than the overnight scope. Agents MUST use `docs/product/preprod-scope.md` as the scope boundary.

## Apply to an existing Courier 404 repository

This v2 archive is designed to overwrite the previous context pack without recreating the original Beads epic.

From the repository root:

```bash
unzip -o courier404_agent_context_v2.zip -d .
chmod +x scripts/*.sh
./scripts/apply_context_v2.sh
```

If Beads has never been initialized in the repository, first use `scripts/bootstrap_beads.sh` from the original/full context flow.

Inspect:

```bash
bd ready
bd blocked
bd list
```

## Night-run entry point

Give the lead agent this instruction:

> Read AGENTS.md first. Run `bd prime`, then read the authoritative product and architecture documents referenced by AGENTS.md. Execute the Courier 404 pre-production vertical slice autonomously from the Beads ready queue using Superpowers. Claim one ready issue at a time, use fresh subagents for implementation and review when the harness supports them, verify before closing, commit every completed logical issue, record newly discovered work in Beads with discovered-from relationships, and continue until there is no ready pre-prod work left. Do not broaden scope, do not stop for routine design choices already resolved by the docs, and leave the repository buildable at every completed checkpoint.

## What is authoritative

Order of precedence:

1. `AGENTS.md`
2. `docs/product/preprod-scope.md`
3. `docs/product/vision.md`
4. `docs/product/gameplay.md`
5. `docs/product/art-direction.md`
6. `docs/architecture/ue5-architecture.md`
7. `docs/architecture/testing-and-verification.md`
8. Beads issue descriptions
9. Superpowers plans

If an implementation plan conflicts with the scope document, the scope document wins.

## Important

This pack does not include Unreal Engine binaries, third-party assets, paid Fab content, or licenses.

If the repository does not yet contain a valid UE5 C++ project, the first Beads task creates one. If UE5 is not installed or cannot be invoked in the agent environment, the agent should record the exact blocker and continue with all source/data/document/test work that can be completed without pretending verification succeeded.


## v2 architecture changes

This revision makes two requirements authoritative:

1. **UE5 feature discipline:** baked/raster baseline, optional Lumen/Nanite/VSM quality layers, explicit asset/frame/package budgets.
2. **Living city:** hierarchical citizen/traffic simulation rather than full AI for all NPCs.

Run `./scripts/apply_context_v2.sh` after unpacking to add the new Beads work and dependencies without duplicating the existing epic.
