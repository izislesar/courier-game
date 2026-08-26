#pragma once

#include "CoreMinimal.h"

/**
 * Deterministic police-stop outcome semantics. Trigger/context comes from the
 * world; the decision depends only on explicit inputs (no omniscient guilt).
 */
enum class ECourier404StopReason : uint8
{
	TrafficViolation,
	SuspiciousArea,
	CargoInspection
};

enum class ECourier404PoliceOutcome : uint8
{
	Warning,
	Fine,
	Arrest
};

struct COURIER404_API FCourier404EncounterInputs
{
	ECourier404StopReason Reason = ECourier404StopReason::TrafficViolation;
	int32 CargoRiskLevel = 0;      // GetPoliceRiskForCargo of carried cargo (0 if empty)
	int32 PriorOffenses = 0;       // warnings + fines + arrests this save
	bool bTriedToFlee = false;
};

struct COURIER404_API FCourier404EncounterResult
{
	ECourier404PoliceOutcome Outcome = ECourier404PoliceOutcome::Warning;
	int32 FineAmount = 0;          // charged on Fine or Arrest
	float DetentionHours = 0.f;    // >0 on Arrest
	bool bFailedActiveContract = false;
};

/**
 * Pure outcome calculator + applier. The caller supplies wallet/clock/contract
 * mutation callbacks so tests stay world-free.
 */
class COURIER404_API FCourier404PoliceEncounter
{
public:
	struct FAppliers
	{
		/** Try to charge Amount; return actually-charged value (clamped to balance). */
		TFunction<int32(int32)> ChargeWallet;
		/** Advance the clock by Hours. */
		TFunction<void(float)> AdvanceClock;
		/** Fail any active contract with the given reason id. */
		TFunction<void(const FName&)> FailActiveContracts;
		/** True when an active risky contract exists (risk>0). */
		TFunction<bool()> HasActiveRiskyContract;
	};

	static ECourier404PoliceOutcome DecideOutcome(const FCourier404EncounterInputs& Inputs);

	/**
	 * Decides and applies consequences through Appliers.
	 * Never leaves the wallet negative; returns exactly what happened.
	 */
	static FCourier404EncounterResult Execute(
		const FCourier404EncounterInputs& Inputs, const FAppliers& Appliers);
};
