#include "Needs/NeedsModel.h"

namespace
{
	constexpr float StarvingThreshold = 0.15f;
}

void FCourier404Needs::Restore(float InHunger01, float InFatigue01, float InHealth01, float InTimeStarvingHours)
{
	Hunger01 = FMath::Clamp(InHunger01, 0.f, 1.f);
	Fatigue01 = FMath::Clamp(InFatigue01, 0.f, 1.f);
	Health01 = FMath::Clamp(InHealth01, 0.f, 1.f);
	TimeStarvingHours = FMath::Max(0.f, InTimeStarvingHours);
}

void FCourier404Needs::Advance(float InHours)
{
	if (InHours <= 0.f)
	{
		return;
	}

	const float PreHunger = Hunger01;

	Hunger01 = FMath::Max(0.f, Hunger01 - HungerDecayPerHour * InHours);
	Fatigue01 = FMath::Max(0.f, Fatigue01 - FatigueDecayPerHour * InHours);

	// Count only the hours actually spent below the starving threshold.
	if (Hunger01 <= StarvingThreshold && !IsDead())
	{
		float HoursStarvingThisStep = InHours;
		if (PreHunger > StarvingThreshold)
		{
			const float HoursToReachStarving =
				FMath::Min(InHours, (PreHunger - StarvingThreshold) / HungerDecayPerHour);
			HoursStarvingThisStep = FMath::Max(0.f, InHours - HoursToReachStarving);
		}
		AccumulateStarving(HoursStarvingThisStep);
	}
	else
	{
		TimeStarvingHours = 0.f; // fed again: grace resets
	}
}

float FCourier404Needs::Eat()
{
	Hunger01 = 1.f;
	TimeStarvingHours = 0.f;
	return Hunger01;
}

void FCourier404Needs::Sleep(float Hours)
{
	if (Hours <= 0.f)
	{
		return;
	}
	Fatigue01 = FMath::Min(1.f, Fatigue01 + FatigueRecoveryPerSleepHour * Hours);
}

ECourier404Hunger FCourier404Needs::GetHunger() const
{
	if (Hunger01 >= 0.7f)
	{
		return ECourier404Hunger::Fed;
	}
	if (Hunger01 >= 0.4f)
	{
		return ECourier404Hunger::Hungry;
	}
	if (Hunger01 > 0.15f)
	{
		return ECourier404Hunger::VeryHungry;
	}
	return ECourier404Hunger::Starving;
}

ECourier404Fatigue FCourier404Needs::GetFatigue() const
{
	if (Fatigue01 >= 0.6f)
	{
		return ECourier404Fatigue::Rested;
	}
	if (Fatigue01 >= 0.25f)
	{
		return ECourier404Fatigue::Tired;
	}
	return ECourier404Fatigue::Exhausted;
}

FCourier404NeedsModifiers FCourier404Needs::ComputeModifiers() const
{
	FCourier404NeedsModifiers Mods;

	switch (GetHunger())
	{
	case ECourier404Hunger::Fed:
	case ECourier404Hunger::Hungry:
		break; // feedback-only: negligible mechanical penalty
	case ECourier404Hunger::VeryHungry:
		Mods.StaminaRecoveryMultiplier *= 0.6f;
		break;
	case ECourier404Hunger::Starving:
		Mods.StaminaRecoveryMultiplier *= 0.3f;
		Mods.MoveSpeedMultiplier *= 0.85f;
		Mods.bAtRisk = true;
		break;
	}

	switch (GetFatigue())
	{
	case ECourier404Fatigue::Rested:
		break;
	case ECourier404Fatigue::Tired:
		Mods.StaminaRecoveryMultiplier *= 0.75f;
		break;
	case ECourier404Fatigue::Exhausted:
		Mods.StaminaRecoveryMultiplier *= 0.5f;
		Mods.MoveSpeedMultiplier *= 0.9f;
		Mods.bAtRisk = true;
		break;
	}

	return Mods;
}

void FCourier404Needs::AccumulateStarving(float InHours)
{
	const float PreGrace = TimeStarvingHours;
	TimeStarvingHours += InHours;

	const float GraceEnd = StarvingGraceHours;
	const float DamageStart = FMath::Max(PreGrace, GraceEnd);
	const float DamagedHours = FMath::Max(0.f, TimeStarvingHours - DamageStart);

	Health01 = FMath::Max(0.f, Health01 - HealthLossPerStarvingHour * DamagedHours);
}