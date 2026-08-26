# Courier 404 — Rendering, Performance and Size Contract

## Status

This document is an architectural contract, not a late optimization checklist.

If an implementation choice conflicts with this document, the implementation must change unless the product owner explicitly changes the contract.

## Product requirement

Courier 404 must look materially better than a 2011-era open-world game while preserving the production discipline that allowed older large games to run on modest hardware.

The project uses Unreal Engine 5, but modern UE5 renderer features are **optional quality multipliers**, not baseline dependencies.

The low-end rendering path must remain a first-class shipping configuration throughout development.

## Reference hardware targets

These are engineering targets, not marketing claims.

### Baseline compatibility target

Representative hardware:

- NVIDIA GeForce GTX 1050
- AMD Radeon RX 560
- four-core desktop/laptop CPU of comparable era
- 8 GB system memory

Target:

- 1600×900 output or lower internal resolution with a clean upscale path
- Low preset
- 30 FPS frame cap
- representative district traversal must remain playable without persistent hitching
- no renderer feature required that excludes this class of GPU

### Primary low-end target

Representative hardware:

- NVIDIA GeForce GTX 1050 Ti
- AMD Radeon RX 570
- 8–16 GB system memory

Target:

- 1920×1080 output
- Low preset
- stable 30 FPS minimum in representative gameplay
- 45 FPS is the optimization goal where scene complexity permits

### Developer/high reference

Representative hardware:

- NVIDIA GeForce RTX 3050 Laptop GPU
- modern 6P/8E-class laptop CPU
- 16 GB system memory

Target:

- 1920×1080
- High preset
- 60 FPS goal in the pre-production district
- editor performance may be lower, but packaged Development/Shipping builds are the performance truth

Do not silently raise the minimum target because a high-end UE5 feature is convenient.

## Renderer architecture

### Baseline renderer

The baseline shipping path must be visually complete with:

- conventional rasterized geometry;
- explicit LODs or validated automatic LODs;
- reproducible low-cost indirect/ambient lighting appropriate to the authoring pipeline;
- baked/static/mixed lighting where it can be generated and shipped reproducibly;
- carefully limited movable lights, including the current generated-District exception documented in `docs/decisions/active-exceptions.md`;
- conventional shadow maps or another non-VSM fallback;
- reflection captures and/or inexpensive screen-space reflections;
- SSAO or equivalent inexpensive ambient contact;
- restrained post-processing;
- texture streaming;
- occlusion/distance culling;
- instancing/HISM/ISM where repetition justifies it.

The game must still communicate its intended atmosphere with all optional expensive UE5 features disabled.

### Lumen

Lumen is allowed as an optional High/Ultra rendering feature.

Rules:

- Lumen must not be required for gameplay readability.
- Lumen must not be the only source of usable indirect light in the authored level.
- The level must be reviewed on a real renderer with Lumen disabled before a lighting task is accepted.
- Lumen hardware ray tracing must never be required by the shipping baseline.
- Prefer software Lumen first when evaluating a higher-quality preset.
- If the Lumen preset causes material/lighting authoring to diverge significantly from the baseline path, remove it rather than maintaining two incompatible art pipelines.

### Nanite

Nanite is allowed selectively.

Good candidates:
- a few complex static hero meshes;
- architectural assets that materially reduce manual LOD burden;
- meshes with a valid fallback representation.

Rules:

- do not enable Nanite as a substitute for asset optimization;
- repeated small props do not become high-poly merely because Nanite exists;
- important Nanite assets must remain acceptable with Nanite disabled/fallback active;
- the level must be smoke-tested with `r.Nanite 0`;
- foliage and masked materials require explicit profiling before Nanite adoption.

### Virtual Shadow Maps

VSM may be used on High/Ultra only if measured.

Rules:

- Low must have a cheaper shadow path;
- do not author the entire scene around ultra-dense dynamic shadows;
- limit shadow-casting movable lights;
- distant/small objects should not consume expensive shadow updates without visible benefit.

### MegaLights and experimental renderer features

Do not make pre-production or shipping architecture depend on experimental renderer features.

MegaLights or equivalent future features may be evaluated in an isolated profiling experiment only.

A feature is not adopted because it exists; it is adopted only after a measured image-quality/performance tradeoff.

## Scalability presets

Every major visual feature must belong to a scalability group or explicit project setting.

### Low

Expected characteristics:

- Lumen off;
- Nanite not required;
- conventional shadow path;
- modest shadow distance/resolution;
- conservative view distance;
- reduced volumetric quality or volumetrics disabled when not compositionally necessary;
- inexpensive reflections;
- reduced effects density;
- conservative texture streaming pool;
- lower crowd/traffic density;
- lower animation update frequency for non-critical actors.

Low is a supported artistic target, not a debug mode.

### Medium

Expected characteristics:

- baseline GI path remains valid;
- improved shadow distance/resolution;
- improved effects and reflection quality;
- moderate crowd/traffic density;
- higher texture quality where memory permits.

Medium should normally provide the best quality/performance ratio.

### High

Expected characteristics:

- optional Lumen preset may be enabled if validated;
- selective Nanite;
- higher-quality shadows/VSM if measured;
- higher effects/reflections;
- target presentation on the RTX 3050 development machine.

### Ultra

Ultra is allowed to spend performance for image quality.

It must not influence minimum requirements or asset authoring discipline.

Ultra may use:
- higher Lumen quality;
- VSM;
- larger shadow distances;
- higher volumetric/reflection quality.

Do not build content that only works on Ultra.

## Frame-time budgets

Measure in a packaged Development build and periodically in Shipping.

### Baseline / Low 30 FPS

Frame budget: 33.3 ms.

Representative traversal target:

- Game Thread: <= 12 ms sustained
- Render Thread: <= 12 ms sustained
- GPU: <= 28 ms sustained
- one-off loading transitions may exceed this, but ordinary street traversal must not produce recurring large stalls

### Developer High 60 FPS

Frame budget: 16.6 ms.

Target on RTX 3050 reference:

- GPU: <= 14.5 ms sustained
- Game Thread: <= 10 ms sustained
- Render Thread: <= 10 ms sustained

Do not optimize exclusively against average FPS. Inspect frame-time graphs and hitching.

## Texture and material budgets

Default texture dimensions:

- small prop: 512 px
- ordinary prop: 1024 px
- large/important prop: 2048 px maximum by default
- 4096 px: exception requiring visible benefit and an issue note
- 8192 px: prohibited for shipping content

Use:
- trim sheets;
- tiling materials;
- decals;
- channel packing;
- material instances;
- shared master materials;
- texture LOD groups;
- streaming mips.

Do not ship source-resolution marketplace textures merely because they were imported.

Initial texture streaming pool targets:

- Low: 768 MB
- Medium: 1200 MB
- High: 1800 MB
- Ultra: 2800 MB

These values are starting budgets and may be tuned downward/upward only from profiling evidence.

A normal gameplay route must not continuously thrash the streaming pool.

## Geometry budgets

Do not define quality by raw triangle count.

Rules:

- every non-Nanite environment mesh used at distance requires valid LOD behavior;
- repeated assets should use instancing where practical;
- merge static geometry only when it improves draw/visibility behavior without destroying culling granularity;
- large buildings should not be one monolithic always-visible mesh if spatial subdivision improves occlusion;
- distant skyline is representation, not simulation;
- interiors not reachable by the player should not carry full interior geometry.

Use `stat RHI`, `stat SceneRendering`, Unreal Insights and GPU profiling to validate scene cost instead of arguing from polygon count alone.

## Material/shader rules

Prefer a small family of master materials:

- opaque surface;
- glass/translucent;
- decal;
- vehicle;
- character where needed.

Use Material Instances for variation.

Avoid:
- unique shader graphs for trivial props;
- unnecessary translucency;
- pixel-depth effects on ordinary surfaces;
- high-cost procedural material functions without profiling;
- per-object dynamic material instances when static parameters are enough.

Every new expensive material feature must be profiled on Low.

## Lighting budget

The city should look excellent because light is art-directed, not because every fixture is fully dynamic.

Rules:

- static environment lighting should prefer baked/static solutions where feasible;
- movable shadow-casting lights are scarce resources;
- decorative lights should often be emissive/material-only or non-shadow-casting;
- police/car headlights may be dynamic because they materially affect gameplay;
- interior lights should be authored for clear culling/attenuation bounds;
- avoid large overlapping movable-light radii.

## Post-processing

Allowed:
- restrained exposure;
- subtle bloom;
- color grading;
- SSAO;
- modest film grain only if artistically justified.

Avoid:
- constant chromatic aberration;
- fake VHS degradation;
- expensive depth-of-field during ordinary gameplay;
- heavy motion blur;
- full-screen effects used to conceal bad lighting.

Low must have a reduced post-process path.

## World construction

The pre-production district should be compact and dense.

Prefer:
- occluding building masses;
- alleys/courtyards;
- bends in streets;
- underpasses;
- interior/exterior transitions;
- skyline proxies;
- audio and lighting that imply a larger city.

Do not keep distant gameplay-grade actors alive merely to imply scale.

World Partition is not mandatory for the pre-production district. Use it only when the actual map size/streaming model justifies its complexity.

## Living-city performance model

The city simulation requirements are defined in `docs/architecture/city-simulation.md`.

Rendering and simulation must share the same distance/importance concept.

Never run full character AI for every visible/background citizen.

## Audio size discipline

- spatial one-shot sounds should normally be mono;
- ambience/music may be stereo;
- use engine-supported lossy compression at an audibly transparent setting;
- stream long music/ambience where appropriate;
- avoid duplicate near-identical source files;
- trim silence;
- source WAV files are production inputs, not a reason to ship uncompressed audio.

## Package-size contract

Targets for cooked packaged content:

### Pre-production vertical slice
- target: <= 3 GiB
- investigate immediately if > 4 GiB

### First public itch.io release
- target: <= 6 GiB
- hard review gate: 8 GiB

Exceeding the review gate requires an explicit asset-size report and product-owner approval.

Do not include:
- unused Starter Content;
- unused plugins;
- editor-only assets;
- source art files;
- duplicate texture sets;
- sample/demo maps;
- marketplace packs that are not used.

Use Unreal packaging compression/IoStore settings appropriate to the engine version and verify the cooked result, not the source repository size.

## Profiling requirements

Optimization is continuous.

Required tools/checks:

- `stat unit`
- `stat gpu`
- `stat streaming`
- `stat memory`
- `memreport -full`
- Unreal Insights
- ProfileGPU / GPU Visualizer
- Asset Audit / Size Map
- packaged Development build

For any performance regression:
1. reproduce;
2. measure;
3. identify dominant cost;
4. fix the dominant cost;
5. measure again.

Do not perform speculative micro-optimization while a larger measured bottleneck exists.

## Continuous performance gates

A feature issue is not complete if it introduces an obvious regression against the relevant budget.

At major integration checkpoints record:

- build configuration;
- resolution/preset;
- representative route;
- average frame time;
- worst recurring hitch;
- Game/Render/GPU times;
- texture streaming status;
- package size;
- observed warnings.

Store reports under `docs/reports/performance/`.

## Acceptance principle

A screenshot taken on Ultra is allowed to look substantially better than Low.

However:

> If disabling Lumen, Nanite and VSM makes the game visually collapse or functionally break, the architecture has failed.

Modern UE5 features enhance Courier 404; they do not define its minimum viable renderer.
