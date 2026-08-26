#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Police/PoliceEncounter.h"
#include "Contracts/SliceContracts.h"
#include "Contracts/ContractDomain.h"
#include "Time/SimClock.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PoliceOutcomes,
	"Courier404.Police.WarningFineArrestPaths",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PoliceOutcomes::RunTest(const FString& Parameters)
{
	// -- Warning: first clean traffic stop --
	{
		FCourier404EncounterInputs In;
		In.Reason = ECourier404StopReason::TrafficViolation;
		int32 Charged = 0;
		FCourier404EncounterResult R = FCourier404PoliceEncounter::Execute(In,
			{ [&Charged](int32 A) { Charged = A; return A; }, nullptr, nullptr, nullptr });
		TestEqual(TEXT("clean first stop warns"), static_cast<int>(R.Outcome), static_cast<int>(ECourier404PoliceOutcome::Warning));
		TestEqual(TEXT("no fine on warning"), R.FineAmount, 0);
		TestEqual(TEXT("wallet untouched"), Charged, 0);
	}

	// -- Fine: repeat offender, ordinary cargo, clamped to balance --
	{
		int32 Balance = 60; // cannot pay a 100 fine
		FCourier404EncounterInputs In;
		In.Reason = ECourier404StopReason::SuspiciousArea;
		In.PriorOffenses = 1;
		FCourier404EncounterResult R = FCourier404PoliceEncounter::Execute(In,
			{ [&Balance](int32 A) { const int32 C = FMath::Min(A, Balance); Balance -= C; return C; },
			  nullptr, nullptr, nullptr });
		TestEqual(TEXT("repeat offense fines"), static_cast<int>(R.Outcome), static_cast<int>(ECourier404PoliceOutcome::Fine));
		TestEqual(TEXT("fine clamped to balance"), R.FineAmount, 60);
		TestEqual(TEXT("wallet never negative"), Balance, 0);
	}

	// -- Arrest: risky cargo + priors; advances time, fails risky work, charges --
	{
		FCourier404SimClock Clock;
		Clock.AdvanceHours(3.f); // 11:00
		int32 Balance = 500;
		FName FailedReason;
		bool bHasActiveRisky = true;

		FCourier404EncounterInputs In;
		In.Reason = ECourier404StopReason::CargoInspection;
		In.CargoRiskLevel = 3; // sealed bag
		In.PriorOffenses = 1;

		FCourier404PoliceEncounter::FAppliers Appliers;
		Appliers.ChargeWallet = [&Balance](int32 A) { const int32 C = FMath::Min(A, Balance); Balance -= C; return C; };
		Appliers.AdvanceClock = [&Clock](float H) { Clock.AdvanceHours(H); };
		Appliers.FailActiveContracts = [&FailedReason](const FName& Why) { FailedReason = Why; };
		Appliers.HasActiveRiskyContract = [&bHasActiveRisky]() { return bHasActiveRisky; };

		const FCourier404EncounterResult R = FCourier404PoliceEncounter::Execute(In, Appliers);
		TestEqual(TEXT("risky cargo + priors arrests"),
			static_cast<int>(R.Outcome), static_cast<int>(ECourier404PoliceOutcome::Arrest));
		TestEqual(TEXT("arrest fine charged"), R.FineAmount, 300);
		TestEqual(TEXT("balance reduced only by fine"), Balance, 200);
		TestEqual(TEXT("detention is eight hours"), R.DetentionHours, 8.f);
		TestEqual(TEXT("clock jumped to 19:00"), Clock.GetHourOfDay(), 19.f);
		TestEqual(TEXT("active risky contract failed"), FailedReason, FName(TEXT("Police.Arrest")));
		TestTrue(TEXT("result reports contract failure"), R.bFailedActiveContract);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PoliceRiskHook,
	"Courier404.Police.RiskHookFromContractRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PoliceRiskHook::RunTest(const FString& Parameters)
{
	FCourier404ContractService Service;
	Courier404SliceContracts::RegisterDefaults(Service);

	TestEqual(TEXT("ordinary parcel clean"), Service.GetPoliceRiskForCargo(TEXT("Cargo.Parcel")), 0);
	TestEqual(TEXT("sealed bag flagged via Rule.PoliceRisk"), Service.GetPoliceRiskForCargo(TEXT("Cargo.SealedBag")), 3);
	TestEqual(TEXT("unknown cargo safe"), Service.GetPoliceRiskForCargo(TEXT("Cargo.Unknown")), 0);

	// Encounter context derives from carried cargo, not from hidden guilt:
	// an empty-handed courier with a clean record gets a warning even while
	// an anonymous job sits accepted in the phone.
	FCourier404EncounterInputs In;
	In.Reason = ECourier404StopReason::TrafficViolation;
	Service.Accept(TEXT("Job.NightDrop"), 0.f); // active risky work exists but nothing carried
	In.CargoRiskLevel = Service.GetPoliceRiskForCargo(NAME_None);
	TestEqual(TEXT("empty hands read as no risk"), In.CargoRiskLevel, 0);
	TestEqual(TEXT("still just a warning"),
		static_cast<int>(FCourier404PoliceEncounter::DecideOutcome(In)),
		static_cast<int>(ECourier404PoliceOutcome::Warning));

	return true;
}

#endif
