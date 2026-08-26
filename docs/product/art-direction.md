# Courier 404 — Art Direction

## Art thesis

A mundane late-2000s / early-2010s urban world rendered with high-end modern UE5 fidelity.

The city should look ordinary, slightly worn and materially believable.

The visual appeal comes from rendering, lighting, composition, atmosphere and detail — not from luxurious objects.

> Mundane objects should look expensive to render, not expensive to own.

## Visual vocabulary

Use:
- aging sedans and hatchbacks;
- apartment blocks;
- concrete stairwells;
- cheap laminate;
- worn paint;
- modest kitchens;
- fluorescent convenience stores;
- parking garages;
- sodium/amber streetlights where appropriate;
- cool fluorescent spill;
- wet asphalt;
- rain residue;
- grime;
- cheap consumer electronics;
- plastic phone mounts;
- paper receipts;
- clutter;
- delivery lockers;
- chain-store signage without real trademark dependence;
- industrial edges of the city.

## Rendering

Target:
- high-quality PBR;
- Lumen-compatible GI/reflections;
- restrained volumetric atmosphere;
- physically plausible material response;
- coherent exposure;
- high-quality shadowing;
- strong practical light sources;
- readable night scenes.

Do not crush blacks so heavily that gameplay becomes unreadable.

## Color and lighting

Avoid a uniform teal/orange grade.

Prefer real-world mixed light:
- warm street lamps;
- cold convenience-store fluorescent light;
- neutral apartment practicals;
- red brake lights;
- white headlights;
- occasional commercial signage.

The world may be dirty without being monochrome.

## Camera

First person.

Goals:
- grounded human height;
- restrained head movement;
- no excessive camera shake;
- no fake bodycam look;
- no fisheye horror lens.

Vehicle interior should feel like a real enclosed personal space.

## UI

The phone UI should resemble a plausible inexpensive contemporary/older smartphone ecosystem, not a sci-fi hacker device.

Use:
- simple typography;
- dark/light practical app screens;
- restrained iconography;
- believable message and courier-app layouts.

Avoid:
- matrix green;
- neon wireframes;
- constant glitch animation;
- skull imagery;
- fake terminal text as decoration.

## Darknet presentation

The hidden economy should feel banal and functional.

A private offer may look almost boring:
- anonymous sender;
- sparse job details;
- large payout;
- coordinates;
- timer;
- terse instructions.

The danger comes from context and consequence.

## World density

One block should have:
- repeated visual landmarks;
- believable clutter;
- signage;
- parked vehicles;
- bins;
- doors;
- utility infrastructure;
- interior glimpses;
- varied light.

Density matters more than geographic scale.

## Characters

Main recurring characters should target believable proportions and contemporary clothing.

The girlfriend must not look like a generic glamour NPC. She should read as a specific ordinary person in the same world.

Hostile street NPCs should not be caricatures.

Police should read as local ordinary law enforcement, not tactical special forces.

## Scalability-aware art direction

The visual identity must not rely on an expensive renderer preset.

Author the scene so that:
- baked/static indirect light carries the baseline composition;
- silhouettes, value structure and practical-light placement remain strong on Low;
- Lumen can enhance High/Ultra without becoming the only reason the scene works;
- material quality comes from correct PBR response, texel discipline and detail placement rather than oversized textures;
- fog, reflections and shadow quality may scale down without destroying navigation/readability.

Before accepting a visual milestone, review at least:
- Low with Lumen/Nanite/VSM dependencies disabled;
- High on the development reference machine.

Do not use expensive UE5 features to compensate for weak composition or poor asset authoring.

## Anti-targets

Do not imitate:
- cyberpunk city art;
- synthwave;
- retro-futurism;
- PSX/PS2 demake presentation;
- VHS overlays;
- horror chromatic-aberration spam;
- neon hacker clichés;
- ultra-clean luxury interiors.

## Placeholder policy

Temporary assets are permitted in pre-prod, but:
- lighting intent should already be strong;
- material scale must be correct;
- collision must be usable;
- final replacement points should be isolated;
- gameplay code must not depend on a specific marketplace asset.

Do not download or redistribute assets without a compatible license.
