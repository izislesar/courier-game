#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Relationship/Relationship.h"
#include "Contracts/SliceContracts.h"
#include "Contracts/ContractDomain.h"
#include "Time/SimClock.h"
#include "UI/PhoneViewModel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404RelMissConsequence,
	"Courier404.Relationship.MissedPlanByClock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404RelMissConsequence::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock; // day 1, 08:00
	FCourier404Relationship Rel;
	Rel.Schedule(1, 20.f);

	TestTrue(TEXT("plan pending"), Rel.HasPendingPlan());

	Clock.AdvanceHours(11.f); // 19:00: window not open yet, plan intact
	Rel.EvaluateMissed(Clock);
	TestFalse(TEXT("not missed before window closes"), Rel.WasLastPlanMissed());

	Clock.AdvanceHours(4.f); // 23:00: window closed
	const bool bMissed = Rel.EvaluateMissed(Clock);
	TestTrue(TEXT("miss recorded by clock"), bMissed);
	TestEqual(TEXT("trust dropped"), Rel.GetTrust(), 0.45f);
	TestEqual(TEXT("miss counter incremented"), Rel.GetMissedCount(), 1);
	TestFalse(TEXT("plan consumed"), Rel.HasPendingPlan());
	TestFalse(TEXT("double evaluation is stable"), Rel.EvaluateMissed(Clock));
	TestEqual(TEXT("trust penalized once only"), Rel.GetTrust(), 0.45f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404RelAttend,
	"Courier404.Relationship.AttendingInsideWindow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404RelAttend::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock;
	FCourier404Relationship Rel;
	Rel.Schedule(1, 20.f);

	Clock.AdvanceHours(12.5f); // 20:30
	TestTrue(TEXT("meeting open now"), Rel.IsMeetingNow(Clock));
	TestTrue(TEXT("attend counted"), Rel.Attend(Clock));
	TestEqual(TEXT("trust rose"), Rel.GetTrust(), 0.85f);
	TestEqual(TEXT("last interaction today"), Rel.GetLastInteractionDay(), 1);
	TestFalse(TEXT("no miss after attending"), Rel.WasLastPlanMissed());
	Rel.EvaluateMissed(Clock);
	TestEqual(TEXT("evaluate stays clean"), Rel.GetTrust(), 0.85f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404RelWorkConflict,
	"Courier404.Relationship.AnonOfferConflictsWithPlan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404RelWorkConflict::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock;
	FCourier404ContractService Service;
	Courier404SliceContracts::RegisterDefaults(Service);
	FCourier404Relationship Rel;
	Rel.Schedule(1, 20.f); // planned evening meeting

	// 19:30: the anonymous offer appears BEFORE the meeting hour.
	Clock.AdvanceHours(11.5f);
	FCourier404PhoneViewModel VM;
	VM.Bind(&Service, &Clock);
	VM.Refresh();

	bool bAnonVisible = false;
	for (const FCourier404PhoneViewModel::FOffer& O : VM.GetOffers())
	{
		bAnonVisible |= O.Category == EContractCategory::Anonymous;
	}
	TestTrue(TEXT("temptation visible pre-meeting"), bAnonVisible);

	// Player takes the risky job; time passes elsewhere.
	const FName Anon = Service.Accept(TEXT("Job.NightDrop"), Clock.GetHourOfDay() * 3600.f);
	Service.MarkPickup(Anon, 0.f);
	Clock.SleepTo(2.f); // delivers through the night, lands day 2 early morning

	// The schedule records what actually happened.
	const bool bMissed = Rel.EvaluateMissed(Clock);
	TestTrue(TEXT("meeting missed while working"), bMissed);
	TestTrue(TEXT("consequence persisted in state"), Rel.WasLastPlanMissed() && Rel.GetMissedCount() == 1);

	// Save/load preserves it (field-level round trip like UPersistenceSubsystem).
	FCourier404Relationship Restored;
	Restored.Restore(Rel.GetTrust(), Rel.GetMissedCount(), Rel.WasLastPlanMissed(),
		Rel.HasPendingPlan(), /*plannedDay*/1, /*plannedHour*/20.f);
	TestTrue(TEXT("miss survives reload"), Restored.WasLastPlanMissed());
	TestEqual(TEXT("trust survives reload"), Restored.GetTrust(), 0.45f);

	return true;
}

#endif
