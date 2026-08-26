#pragma once

#include "CoreMinimal.h"

enum class ECourier404Hunger : uint8
{
	Fed,
	Hungry,
	VeryHungry,
	Starving
};

enum class ECourier404Fatigue : uint8
{
	Rested,
	Tired,
	Exhausted
};

/** Mechanical outputs consumed by movement/stamina systems. */
struct COURIER404_API FCourier404NeedsModifiers
{
	float StaminaRecoveryMultiplier = 1.f;
	float MoveSpeedMultiplier = 1.f;
	bool bAtRisk = false; // starving or exhausted: hooks for police/hostile/driving risk
};

/**
 * Deterministic needs simulation. One missed meal or one late night must never
 * be catastrophic; starvation kills only after prolonged neglect.
 *
 * All values are 0..1 floats: Hunger01 1=fed..0=starved, Fatigue01 1=rested,
 * Health01 1=healthy..0=dead.
 */
class COURIER404_API FCourier404Needs
{
public:
	static constexpr float HungerDecayPerHour = 1.f / 48.f;
	static constexpr float FatigueDecayPerHour = 1.f / 40.f;
	static constexpr float FatigueRecoveryPerSleepHour = 1.f / 8.f;
	static constexpr float StarvingGraceHours = 12.f;
	static constexpr float HealthLossPerStarvingHour = 0.06f;

	void Advance(float InHours);

	/** Eating restores hunger fully (a meal). Returns new hunger value. */
	float Eat();

	/** Sleeping restores fatigue proportional to hours and pauses hunger decay during rest. */
	void Sleep(float Hours);

	ECourier404Hunger GetHunger() const;
	ECourier404Fatigue GetFatigue() const;

	float GetHunger01() const { return Hunger01; }
	float GetFatigue01() const { return Fatigue01; }
	float GetHealth01() const { return Health01; }
	bool IsDead() const { return Health01 <= 0.f; }
	float GetTimeStarving() const { return TimeStarvingHours; }

	FCourier404NeedsModifiers ComputeModifiers() const;

private:
	void AccumulateStarving(float InHours);

	float Hunger01 = 1.f;
	float Fatigue01 = 1.f;
	float Health01 = 1.f;
	float TimeStarvingHours = 0.f;
};
