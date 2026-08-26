#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Encounters/Incapacitation.h"
#include "Contracts/SliceContracts.h"
#include "Contracts/ContractDomain.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404DeathUnifiedRecovery,
	"Courier404.Death.UnifiedStableRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404DeathUnifiedRecovery::RunTest(const FString& Parameters)
{
	// -- Violence death (hostile beating) --
	FCourier404SimClock Clock; // day 1
	Clock.AdvanceHours(20.f);  // 04:00 next-day window anyway

	FCourier404Wallet Wallet;
	Wallet.Add(250);

	FCourier404Needs Needs;
	Needs.Restore(/*hunger*/0.8f, /*fatigue*/0.3f, /*health*/0.0f, 0.f); // beaten to death
	FName FailedReason;
	bool bHadActive = true;

	FCourier404Incapacitation::FHooks Hooks;
	Hooks.HasActiveContract = [&bHadActive]() { return bHadActive; };
	Hooks.FailActiveContracts = [&FailedReason](const FName& Why) { FailedReason = Why; };

	const FCourier404DeathRecovery R = FCourier404Incapacitation::Recover(
		ECourier404DeathSource::Violence, Wallet, Needs, Clock, Hooks);

	TestEqual(TEXT("medical charged"), R.MedicalCost, 100);
	TestEqual(TEXT("wallet reduced only by medical"), Wallet.GetBalance(), 150);
	TestEqual(TEXT("wake next morning 08:00"), R.WakeHour, 8.f);
	TestTrue(TEXT("day advanced for recovery"), R.WakeDayIndex >= 2);
	TestEqual(TEXT("health partially restored"), Needs.GetHealth01(), 0.5f);
	TestEqual(TEXT("no longer starving timer"), Needs.GetTimeStarving(), 0.f);
	TestEqual(TEXT("active work failed with reason"), FailedReason, FName(TEXT("Player.Incapacitated")));
	TestTrue(TEXT("recovery reports contract failure"), R.bFailedActiveContract);

	// -- Starvation path: neglect reaches IsDead then same recovery applies --
	FCourier404SimClock ClockS;
	{
		FCourier404Wallet W2;
		W2.Add(30); // poor: medical clamps to 30
		FCourier404Needs N2;
		for (int32 Hour = 0; Hour < 200 && !N2.IsDead(); ++Hour)
		{
			N2.Advance(1.f);
		}
		TestTrue(TEXT("prolonged starvation is lethal"), N2.IsDead());

		FCourier404SimClock CS;
		FCourier404ContractService Svc;
		Courier404SliceContracts::RegisterDefaults(Svc);
		Svc.Accept(TEXT("Job.Courier01"), 0.f);

		FCourier404Incapacitation::FHooks SHooks;
		SHooks.HasActiveContract = [&Svc]() { return !Svc.FindActiveInstanceForCargo(TEXT("Cargo.Parcel")).IsNone(); };
		SHooks.FailActiveContracts = [&Svc](const FName& Why)
		{
			const FName Id = Svc.FindActiveInstanceForCargo(TEXT("Cargo.Parcel"));
			if (!Id.IsNone()) { Svc.Fail(Id, Why, 0.f); }
		};

		const FCourier404DeathRecovery RS = FCourier404Incapacitation::Recover(
			ECourier404DeathSource::Starvation, W2, N2, CS, SHooks);

		TestEqual(TEXT("poor victim pays what they have"), RS.MedicalCost, 30);
		TestEqual(TEXT("wallet never negative"), W2.GetBalance(), 0);
		TestTrue(TEXT("starvation death fails active work"), RS.bFailedActiveContract);

		// Stability: recovered body is alive and functional.
		N2.Advance(24.f);
		TestFalse(TEXT("recovered body not instantly dying again"), N2.IsDead());
	}

	return true;
}

#endif
