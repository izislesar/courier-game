#pragma once

#include "CoreMinimal.h"
#include "Time/SimClock.h"

/**
 * Hierarchical living-city baseline (docs/architecture/city-simulation.md):
 * Tier 0 persistent records (data only), Tier 1 budgeted lightweight agents,
 * Tier 2 full NPCs are the existing gameplay actors (girlfriend, hostile,
 * police). Everything here is deterministic plain C++.
 */

enum class ECourier404Activity : uint8
{
	Home,
	Commute,
	Shop,
	Sit,
	Smoke,
	Walk,
	Sleep,
	React
};

struct COURIER404_API FCourier404CitizenRecord
{
	FName Id;
	FName HomeZone;
	FName CurrentZone;
	ECourier404Activity Activity = ECourier404Activity::Home;
	bool bFledRecently = false;
	int32 LastSimDay = 1;
};

/** Tier-0 population: hundreds of records, zero actors. */
class COURIER404_API FCourier404CityRoster
{
public:
	static constexpr int32 TargetRecords = 250;

	/** Deterministic roster generation (fixed seed). */
	void Generate(int32 Count = TargetRecords);

	const TArray<FCourier404CitizenRecord>& GetRecords() const { return Records; }

	/** Reassigns activities/zones per time-of-day profile. Deterministic. */
	void EvaluateTimeOfDay(ECourier404DayPhase Phase);

	/** Disturbance reaction: citizens in ZoneId flee; others unaffected. */
	int32 TriggerDisturbance(const FName& ZoneId);

	/** Non-mission ambient event: courier unloading at Parking, midday only. */
	static bool IsAmbientEventActive(ECourier404DayPhase Phase, float HourOfDay);

	/** Tier-1 agent budget by scalability level (0=Low..3=Ultra). */
	static int32 GetAmbientBudget(ECourier404DayPhase Phase, int32 ScalabilityLevel);

private:
	TArray<FCourier404CitizenRecord> Records;
};

/**
 * Tier-1 agent pool: recycles a bounded set of lightweight placeholder actors
 * according to the budget. Off-screen citizens never require these actors;
 * records remain authoritative either way.
 */
class COURIER404_API FCourier404AgentPool
{
public:
	void SyncToBudget(int32 Budget, UWorld* World);
	int32 GetSpawnedCount() const { return Agents.Num(); }

private:
	TArray<TWeakObjectPtr<class AActor>> Agents;
};