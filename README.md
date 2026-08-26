# courier-game

Courier 404 — first-person urban courier/life simulation in Unreal Engine 5.8.2.

The pre-production vertical slice has been proven; active development is now **production readiness**: player feel, presentation, reliability, packaged performance, save integrity, living-city quality and shipping discipline.

## Build

See `docs/architecture/build-commands.md`. Use the engine recorded in `.ue-env`; never replace the UE bundled toolchain with system clang.

## Agent/task context

- Agent operating manual: `AGENTS.md`
- Current task state: Beads (`bd prime`, `bd ready --json`)
- Production acceptance boundary: `docs/product/production-readiness.md`
- Context recovery: `docs/agent/context-loading.md`
- Autonomous run protocol: `docs/agent/night-run.md`

Historical pre-production plans/reports are evidence only and must not override live Beads state.
