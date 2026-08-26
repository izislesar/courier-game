# Active Architecture Exceptions

These are explicit, temporary or scoped exceptions to preferred architecture. They exist to prevent agents from rediscovering the same conflict or silently turning a workaround into an unlimited invariant.

## Generated District01 lighting

Current state:
- District01 is deterministically authored by commandlet;
- the current rig uses movable light components so packaged/runtime lighting does not depend on stale or absent baked build data;
- structural audits require the generated runtime lights to persist and remain valid.

Constraint:
- this does **not** authorize unrestricted movable/shadow-casting lights;
- light count, shadow cost and GPU budget remain governed by `performance-budget.md`;
- production lighting tasks must include real-renderer visual review;
- if baked/static/mixed lighting can later be introduced reproducibly without breaking deterministic authoring, evaluate it through a measured Beads issue rather than changing the contract ad hoc.

## Vehicle implementation

Current slice uses the lightweight drivetrain behind the vehicle facade.

Chaos Vehicles replacement remains an asset-dependent backlog concern. Gameplay systems must depend on the facade, not on the temporary drivetrain implementation.
