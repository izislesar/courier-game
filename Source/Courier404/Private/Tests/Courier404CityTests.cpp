#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "City/CitySimulation.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404CityTier0,
	"Courier404.City.Tier0RecordsAndTimeOfDay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404CityTier0::RunTest(const FString& Parameters)
{
	FCourier404CityRoster Roster;
	Roster.Generate();
	TestEqual(TEXT("250 persistent records"), Roster.GetRecords().Num(), 250);

	// Determinism: identical regeneration.
	FCourier404CityRoster Other;
	Other.Generate();
	bool bIdentical = true;
	for (int32 i = 0; i < 250; ++i)
	{
		bIdentical &= Other.GetRecords()[i].Id == Roster.GetRecords()[i].Id;
	}
	TestTrue(TEXT("generation deterministic"), bIdentical);

	// Time-of-day: Day profile shows at least three ambient activities
	// (shop, sit, smoke) and night mostly sleeps.
	Roster.EvaluateTimeOfDay(ECourier404DayPhase::Day);
	TSet<ECourier404Activity> DayActivities;
	int32 FledBefore = 0;
	for (const FCourier404CitizenRecord& R : Roster.GetRecords())
	{
		DayActivities.Add(R.Activity);
		FledBefore += R.bFledRecently ? 1 : 0;
	}
	TestTrue(TEXT("day has shopping"), DayActivities.Contains(ECourier404Activity::Shop));
	TestTrue(TEXT("day has sitting"), DayActivities.Contains(ECourier404Activity::Sit));
	TestTrue(TEXT("day has smoking"), DayActivities.Contains(ECourier404Activity::Smoke));

	Roster.EvaluateTimeOfDay(ECourier404DayPhase::Night);
	int32 Sleeping = 0;
	for (const FCourier404CitizenRecord& R : Roster.GetRecords())
	{
		Sleeping += R.Activity == ECourier404Activity::Sleep ? 1 : 0;
	}
	TestTrue(TEXT("night feels quieter"), Sleeping > 150);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404CityDensityAndEvents,
	"Courier404.City.DensityScalesLowAndDisturbanceReacts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404CityDensityAndEvents::RunTest(const FString& Parameters)
{
	// Budget monotonicity + Low reduction.
	const int32 LowDay = FCourier404CityRoster::GetAmbientBudget(ECourier404DayPhase::Day, 0);
	const int32 HighDay = FCourier404CityRoster::GetAmbientBudget(ECourier404DayPhase::Day, 2);
	TestTrue(TEXT("Low below High"), LowDay < HighDay);
	TestTrue(TEXT("night thins out everywhere"),
		FCourier404CityRoster::GetAmbientBudget(ECourier404DayPhase::Night, 2) < HighDay);
	const int32 EveningUltra = FCourier404CityRoster::GetAmbientBudget(ECourier404DayPhase::Evening, 3);
	TestTrue(TEXT("evening ultra within documented ceiling"), EveningUltra <= 60 && EveningUltra > HighDay);

	// Non-mission ambient event window: courier unloading midday only.
	TestFalse(TEXT("no unload event morning"),
		FCourier404CityRoster::IsAmbientEventActive(ECourier404DayPhase::Morning, 9.f));
	TestTrue(TEXT("unload event midday"),
		FCourier404CityRoster::IsAmbientEventActive(ECourier404DayPhase::Day, 14.f));

	// Disturbance reaction: same-zone citizens react, others do not.
	FCourier404CityRoster Roster;
	Roster.Generate();
	Roster.EvaluateTimeOfDay(ECourier404DayPhase::Day);

	const FName TargetZone = TEXT("Zone.South");
	int32 InZone = 0;
	for (const FCourier404CitizenRecord& R : Roster.GetRecords())
	{
		InZone += R.CurrentZone == TargetZone ? 1 : 0;
	}
	const int32 Reacted = Roster.TriggerDisturbance(TargetZone);
	TestEqual(TEXT("all in-zone citizens reacted"), Reacted, InZone);

	int32 StillReacting = 0;
	for (const FCourier404CitizenRecord& R : Roster.GetRecords())
	{
		if (R.bFledRecently)
		{
			StillReacting++;
			TestEqual(TEXT("reaction is flee activity"),
				static_cast<int>(R.Activity), static_cast<int>(ECourier404Activity::React));
		}
	}
	TestEqual(TEXT("reactions bounded to zone"), StillReacting, InZone);

	// Tier-1 pool: live placeholder agents follow the budget exactly (Low safe).
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("world"), World))
	{
		return false;
	}
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	FCourier404AgentPool Pool;
	Pool.SyncToBudget(HighDay, World);
	TestEqual(TEXT("pool fills to high budget"), Pool.GetSpawnedCount(), HighDay);

	Pool.SyncToBudget(LowDay, World); // scalability drops to Low mid-session
	TestTrue(TEXT("pool shrinks safely on Low"), Pool.GetSpawnedCount() == LowDay && Pool.GetSpawnedCount() < HighDay);

	// Records remain authoritative regardless of actors: roster untouched.
	TestEqual(TEXT("records unaffected by actor churn"), Roster.GetRecords().Num(), 250);

	World->DestroyWorld(false);
	return true;
}

#endif
