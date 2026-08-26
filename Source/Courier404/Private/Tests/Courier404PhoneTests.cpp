#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Contracts/ContractDomain.h"
#include "Time/SimClock.h"
#include "UI/PhoneViewModel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PhoneOffers,
	"Courier404.Phone.OffersAndAcceptReject",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PhoneOffers::RunTest(const FString& Parameters)
{
	FCourier404ContractService Service;
	FCourier404SimClock Clock; // 08:00 day

	FContractDefinition Normal;
	Normal.ContractId = TEXT("Job.AAA");
	Normal.PickupPointId = TEXT("Point.Store");
	Normal.DropoffPointId = TEXT("Point.DropA");
	Normal.Reward = 120;
	Normal.CargoId = TEXT("Cargo.Parcel");
	Normal.TimeLimitSeconds = 600.f;
	TArray<FString> Errors;
	TestTrue(TEXT("normal registered"), Service.RegisterDefinition(Normal, Errors));

	FCourier404PhoneViewModel VM;
	VM.Bind(&Service, &Clock);
	VM.Refresh();
	TestEqual(TEXT("one normal offer at morning"), VM.GetOffers().Num(), 1);

	// Anonymous offer is hidden during the day.
	FContractDefinition Anon = Normal;
	Anon.ContractId = TEXT("Job.ZZ");
	Anon.Category = EContractCategory::Anonymous;
	Anon.Reward = 900;
	Anon.RiskLevel = 3;
	TestTrue(TEXT("anon registered"), Service.RegisterDefinition(Anon, Errors));
	VM.Refresh();
	TestEqual(TEXT("anonymous hidden by day"), VM.GetOffers().Num(), 1);

	Clock.SleepTo(23.f); // night
	VM.Refresh();
	TestEqual(TEXT("anonymous visible at night"), VM.GetOffers().Num(), 2);

	// Selection + accept: creates exactly one active instance and records a message.
	VM.MoveSelection(5); // clamps to last (anonymous)
	TestTrue(TEXT("selected anonymous"), VM.GetOffers()[VM.GetSelectedIndex()].Category == EContractCategory::Anonymous);

	const FName Instance = VM.AcceptSelected();
	TestFalse(TEXT("accept created instance"), Instance.IsNone());
	TestEqual(TEXT("active id set"), VM.GetActiveInstanceId(), Instance);
	TestEqual(TEXT("exactly one instance"), Service.GetDefinitions().Num(), 2);
	TestTrue(TEXT("message recorded"), VM.GetMessages().Num() == 1);

	// Single-active-job rule: while a job is open, the board is empty.
	VM.Refresh();
	TestEqual(TEXT("board empty while job active"), VM.GetOffers().Num(), 0);
	TestTrue(TEXT("objective tracks active job"), VM.GetObjectiveLine().Contains(TEXT("Point.Store")));

	// Complete the active job properly, then the board re-opens.
	Service.MarkPickup(Instance, 90.f);
	int32 Payout = 0;
	TestTrue(TEXT("delivery succeeds"), Service.TryDeliver(Instance, TEXT("Point.DropA"), 100.f, Payout));
	VM.Refresh();
	TestEqual(TEXT("normal job re-offered after completion"), VM.GetOffers().Num(), 1);

	// Objective reflects the finished job as feedback once nothing is open.
	VM.SetScreen(FCourier404PhoneViewModel::EScreen::ActiveJob);
	TestTrue(TEXT("objective shows completion"), VM.GetObjectiveLine().Contains(TEXT("Delivered")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PhoneObjectiveStates,
	"Courier404.Phone.ObjectiveTracksContractState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PhoneObjectiveStates::RunTest(const FString& Parameters)
{
	FCourier404ContractService Service;
	FCourier404SimClock Clock;

	FContractDefinition Def;
	Def.ContractId = TEXT("Job.Obj");
	Def.PickupPointId = TEXT("Point.Store");
	Def.DropoffPointId = TEXT("Point.DropA");
	Def.Reward = 80;
	Def.CargoId = TEXT("Cargo.Parcel");
	TArray<FString> Errors;
	Service.RegisterDefinition(Def, Errors);

	FCourier404PhoneViewModel VM;
	VM.Bind(&Service, &Clock);
	VM.Refresh();

	TestEqual(TEXT("idle objective before accept"), VM.GetObjectiveLine(), FString(TEXT("No active job")));

	const FName Instance = Service.Accept(TEXT("Job.Obj"), 0.f);
	VM.SetScreen(FCourier404PhoneViewModel::EScreen::ActiveJob);
	TestTrue(TEXT("pickup objective after accept"), VM.GetObjectiveLine().Contains(TEXT("Pick up")));

	Service.MarkPickup(Instance, 10.f);
	TestTrue(TEXT("delivery objective after pickup"), VM.GetObjectiveLine().Contains(TEXT("Deliver to")));

	int32 Payout = 0;
	Service.TryDeliver(Instance, TEXT("Point.DropA"), 20.f, Payout);
	TestTrue(TEXT("completion objective"), VM.GetObjectiveLine().Contains(TEXT("+80")));

	return true;
}

#endif
