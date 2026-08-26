#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Contracts/SliceContracts.h"
#include "Persistence/Courier404SaveGame.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PersistSlotIO,
	"Courier404.Persistence.SaveSlotRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PersistSlotIO::RunTest(const FString& Parameters)
{
	UCourier404SaveGame* Out = NewObject<UCourier404SaveGame>();
	Out->Money = 321;
	Out->DayIndex = 4;
	Out->HourOfDay = 21.5f;
	Out->Hunger01 = 0.42f;

	const FString Slot = FString::Printf(TEXT("Courier404_Auto_%u"), FPlatformTime::Cycles() & 0xFFFF);
	TestTrue(TEXT("save to slot"), UGameplayStatics::SaveGameToSlot(Out, Slot, 0));

	UCourier404SaveGame* In = Cast<UCourier404SaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
	if (TestNotNull(TEXT("loaded save"), In))
	{
		TestEqual(TEXT("slot money"), In->Money, 321);
		TestEqual(TEXT("slot day"), In->DayIndex, 4);
		TestEqual(TEXT("slot hour"), In->HourOfDay, 21.5f);
		TestEqual(TEXT("slot hunger"), In->Hunger01, 0.42f);
	}
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	return true;
}

// Domain-level capture/apply mirroring UPersistenceSubsystem semantics.
#include "Economy/Wallet.h"
#include "Core/LifeService.h"
#include "Time/SimClock.h"
#include "Needs/NeedsModel.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PersistRoundTrip,
	"Courier404.Persistence.CaptureApplyRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PersistRoundTrip::RunTest(const FString& Parameters)
{
	// -- Live state A: mid-game --
	FCourier404SimClock ClockA;
	FCourier404LifeService LifeA(&ClockA);
	LifeA.GetWallet().Add(50);
	FCourier404ContractService ContractsA;
	Courier404SliceContracts::RegisterDefaults(ContractsA);

	int32 Payouts = 0;
	ContractsA.OnContractCompleted.AddLambda([&](const FContractRuntimeState&, int32) { ++Payouts; });

	const FName Instance = ContractsA.Accept(TEXT("Job.Courier01"), 5.f);
	ContractsA.MarkPickup(Instance, 30.f);
	LifeA.GetWallet().Add(120); // simulate an earlier payout
	LifeA.Advance(14.f);        // degrade needs
	ClockA.AdvanceHours(19.f);  // day 2, 03:00

	UCourier404SaveGame* Saved = NewObject<UCourier404SaveGame>();
	Saved->Money = LifeA.GetWallet().GetBalance();
	Saved->DayIndex = ClockA.GetDayIndex();
	Saved->HourOfDay = ClockA.GetHourOfDay();
	Saved->Hunger01 = LifeA.GetNeeds().GetHunger01();
	Saved->Fatigue01 = LifeA.GetNeeds().GetFatigue01();
	Saved->Health01 = LifeA.GetNeeds().GetHealth01();
	Saved->TimeStarvingHours = LifeA.GetNeeds().GetTimeStarving();
	for (const TPair<FName, FContractRuntimeState>& Pair : ContractsA.GetInstances())
	{
		FCourier404ContractRecord R;
		R.InstanceId = Pair.Value.InstanceId;
		R.ContractId = Pair.Value.ContractId;
		R.Status = Pair.Value.Status;
		R.AcceptedAtSimSeconds = Pair.Value.AcceptedAtSimSeconds;
		R.PickedUpAtSimSeconds = Pair.Value.PickedUpAtSimSeconds;
		R.FailureReason = Pair.Value.FailureReason;
		Saved->ContractRecords.Add(R);
	}

	// -- Fresh state B (post-reload): apply and verify --
	FCourier404SimClock ClockB;
	FCourier404LifeService LifeB(&ClockB);
	FCourier404ContractService ContractsB;
	Courier404SliceContracts::RegisterDefaults(ContractsB);

	int32 PayoutsB = 0;
	ContractsB.OnContractCompleted.AddLambda([&](const FContractRuntimeState&, int32) { ++PayoutsB; });

	for (const FCourier404ContractRecord& R : Saved->ContractRecords)
	{
		FContractRuntimeState S;
		S.InstanceId = R.InstanceId;
		S.ContractId = R.ContractId;
		S.Status = R.Status;
		S.AcceptedAtSimSeconds = R.AcceptedAtSimSeconds;
		S.PickedUpAtSimSeconds = R.PickedUpAtSimSeconds;
		S.CompletedAtSimSeconds = R.CompletedAtSimSeconds;
		S.FailedAtSimSeconds = R.FailedAtSimSeconds;
		S.FailureReason = R.FailureReason;
		ContractsB.RestoreInstance(S);
	}
	LifeB.GetWallet().SetBalance(Saved->Money);
	ClockB.Restore(Saved->DayIndex, Saved->HourOfDay);
	LifeB.GetNeeds().Restore(Saved->Hunger01, Saved->Fatigue01, Saved->Health01, Saved->TimeStarvingHours);

	TestEqual(TEXT("money restored"), LifeB.GetWallet().GetBalance(), 170);
	TestEqual(TEXT("day restored"), ClockB.GetDayIndex(), 2);
	TestEqual(TEXT("hour restored"), ClockB.GetHourOfDay(), ClockA.GetHourOfDay());
	TestEqual(TEXT("hunger restored"), LifeB.GetNeeds().GetHunger01(), LifeA.GetNeeds().GetHunger01());

	// Restored picked-up job resumes correctly and pays EXACTLY once post-reload.
	int32 Payout = 0;
	TestTrue(TEXT("restored pickup completes"), ContractsB.TryDeliver(Instance, TEXT("Point.DropA"), 1.f, Payout));
	TestEqual(TEXT("paid once after reload"), PayoutsB, 1);

	int32 Second = 0;
	TestFalse(TEXT("no duplicate reward"), ContractsB.TryDeliver(Instance, TEXT("Point.DropA"), 2.f, Second));
	TestEqual(TEXT("still one payout event"), PayoutsB, 1);

	return true;
}

#endif
