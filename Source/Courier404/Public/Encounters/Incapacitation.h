#pragma once

#include "CoreMinimal.h"
#include "Economy/Wallet.h"
#include "Needs/NeedsModel.h"
#include "Time/SimClock.h"

enum class ECourier404DeathSource : uint8
{
	Violence,
	Collision,
	Starvation
};

struct COURIER404_API FCourier404DeathRecovery
{
	ECourier404DeathSource Source = ECourier404DeathSource::Starvation;
	int32 MedicalCost = 0;
	int32 WakeDayIndex = 1;
	float WakeHour = 8.f;
	bool bFailedActiveContract = false;
};

/**
 * Unified incapacitation/restart semantics. Whatever killed the player,
 * recovery is identical and stable: wake at home next morning, medical cost
 * (clamped to balance), partial health, active work failed.
 */
class COURIER404_API FCourier404Incapacitation
{
public:
	static constexpr int32 MedicalCostMax = 100;

	struct FHooks
	{
		/** Fail any active contract with the given reason id. */
		TFunction<void(const FName&)> FailActiveContracts;
		/** True when an active contract exists (any status Accepted/PickedUp). */
		TFunction<bool()> HasActiveContract;
	};

	static bool IsLethal(const FCourier404Needs& Needs)
	{
		return Needs.IsDead();
	}

	/**
	 * Applies death consequences and returns the recovery summary.
	 * Safe to call only once per death event; caller gates on Needs.IsDead().
	 */
	static FCourier404DeathRecovery Recover(
		ECourier404DeathSource Source,
		FCourier404Wallet& Wallet,
		FCourier404Needs& Needs,
		FCourier404SimClock& Clock,
		const FHooks& Hooks);
};
