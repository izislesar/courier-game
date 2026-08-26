# Courier 404 — Gameplay Rules

## Core loop

Normal life loop:

```text
wake
→ check phone
→ choose work
→ travel
→ pickup
→ transport
→ deliver
→ get paid
→ spend / eat / maintain life
→ relationship / personal time
→ sleep
→ next day
```

Risk loop:

```text
anonymous offer
→ unusually high reward
→ accept or reject
→ pickup
→ travel with risk
→ police / hostile / time pressure possibility
→ deliver or fail
→ consequence
```

## Design principle: routine with delayed consequences

Routine is optional in the short term but neglect compounds.

Bad design:
- hunger decreases every minute and demands constant eating.

Desired design:
- missing one meal barely matters;
- repeated neglect changes stamina and recovery;
- prolonged starvation is lethal.

The same principle applies to sleep.

## Hunger

Suggested domain model:

```text
Fed
Hungry
VeryHungry
Starving
```

Effects:

- `Fed`: normal.
- `Hungry`: primarily feedback; negligible mechanical penalty.
- `VeryHungry`: reduced stamina recovery.
- `Starving`: stronger stamina penalty and gradual health loss after a grace period.

Do not make food consumption a frequent chore.

## Fatigue

```text
Rested
Tired
Exhausted
```

Effects:

- `Rested`: normal.
- `Tired`: mild stamina/reaction penalty.
- `Exhausted`: stronger stamina impact and optional driving-risk hooks.

Microsleep is a possible future system, not required unless trivial to add safely.

## Health/injury

Health is not intended as an arcade-combat loop.

Damage sources:
- hostile NPC;
- vehicle impact;
- future accidents;
- starvation.

Injury consequence may include:
- reduced movement/stamina;
- money loss;
- time loss;
- recovery after sleep/time.

## Money

Money creates pressure, not spreadsheet management.

Income:
- normal delivery;
- anonymous delivery.

Pre-prod expenses:
- food;
- fine;
- optional incident/medical consequence;
- fuel if implemented.

## Contracts

Contracts must be content-data, not hard-coded one-off mission graphs.

A contract definition should support fields conceptually similar to:

```text
id
category
pickup
dropoff
reward
availability_window
time_limit
cargo
risk
rules[]
consequence_profile
```

Example modifiers:
- fragile;
- time-limited;
- destination changes after pickup;
- anonymous sender;
- high police risk.

The pre-prod slice does not need every modifier.

## Failure

A contract may fail due to:
- abandonment;
- time expiration;
- arrest;
- cargo loss;
- player incapacitation/death;
- explicit invalid delivery.

Failure must produce a consequence but must not corrupt future progression.

## Relationship

Relationship state is not a simple visible affection meter.

Persist concepts such as:
- trust/strain;
- last interaction;
- missed plan;
- current availability.

The slice needs one meaningful scheduling conflict.

Example:
- planned meeting at 20:00;
- anonymous offer appears at 19:30;
- player physically chooses where to go;
- later message/state reflects the choice.

No modal morality choice.

## Police

Police should feel mundane and procedural rather than omniscient.

The system should not know the player is guilty merely because a hidden contract is active.

A stop can originate from ordinary behavior.

Potential outcome decision factors:
- reason for stop;
- active risky contract;
- current carried cargo;
- prior status;
- configured randomness only if deterministic/replay-safe for tests.

## Hostile NPCs

Hostile NPCs exist to make the city socially dangerous, not to create a beat-em-up.

Desired behavior:
- warning/threat;
- approach;
- attack if escalation continues;
- player can disengage/flee;
- multiple attackers are dangerous.

Losing can produce:
- money theft;
- contract loss;
- injury;
- time skip;
- death in severe path.

## Driving

Driving should be readable and satisfying, not simulation-heavy.

Target:
- ordinary used car;
- believable weight;
- accessible steering;
- no racing-game tuning;
- collisions matter.

Do not build transmission simulation, tire temperatures or detailed mechanical damage in pre-prod.

## Diegetic presentation

Prefer:
- phone for jobs/messages;
- dashboard/in-world feedback;
- actual food interaction;
- physical package;
- bed to sleep;
- police actor interaction.

Use HUD only where diegetic display would harm clarity.
