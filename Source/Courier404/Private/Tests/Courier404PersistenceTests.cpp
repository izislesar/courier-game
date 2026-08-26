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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PersistFutureVersionRejected,
	"Courier404.Persistence.FutureVersionRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PersistFutureVersionRejected::RunTest(const FString& Parameters)
{
	UCourier404SaveGame* Future = NewObject<UCourier404SaveGame>();
	Future->SaveVersion = UCourier404SaveGame::CurrentVersion + 100;
	Future->Money = 9999;

	UCourier404SaveGame* Current = NewObject<UCourier404SaveGame>();
	Current->SaveVersion = UCourier404SaveGame::CurrentVersion;

	TestTrue(TEXT("future version detected as incompatible"),
		Future->SaveVersion != UCourier404SaveGame::CurrentVersion);
	TestFalse(TEXT("current version detected as compatible"),
		Current->SaveVersion != UCourier404SaveGame::CurrentVersion);

	FCourier404SimClock Clock;
	FCourier404LifeService Life(&Clock);
	Life.GetWallet().Add(42);
	const int32 MoneyBefore = Life.GetWallet().GetBalance();
	const int32 DayBefore = Clock.GetDayIndex();

	const bool bWouldApply = (Future->SaveVersion == UCourier404SaveGame::CurrentVersion);
	TestFalse(TEXT("future version refused by version gate"), bWouldApply);
	TestEqual(TEXT("money unchanged after refusal"), Life.GetWallet().GetBalance(), MoneyBefore);
	TestEqual(TEXT("day unchanged after refusal"), Clock.GetDayIndex(), DayBefore);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PersistCorruptSaveHandled,
	"Courier404.Persistence.CorruptSaveHandled",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PersistCorruptSaveHandled::RunTest(const FString& Parameters)
{
	UCourier404SaveGame* Corrupt = NewObject<UCourier404SaveGame>();
	Corrupt->SaveVersion = UCourier404SaveGame::CurrentVersion;
	Corrupt->Money = -500;
	Corrupt->Hunger01 = -1.f;
	Corrupt->Fatigue01 = 2.f;
	Corrupt->Health01 = -0.5f;
	Corrupt->TimeStarvingHours = -10.f;
	Corrupt->DayIndex = -3;
	Corrupt->HourOfDay = 48.f;

	FCourier404SimClock Clock;
	FCourier404LifeService Life(&Clock);
	Life.GetWallet().Add(200);

	Life.GetWallet().SetBalance(Corrupt->Money);
	TestEqual(TEXT("negative money rejected by wallet"), Life.GetWallet().GetBalance(), 200);

	Life.GetNeeds().Restore(Corrupt->Hunger01, Corrupt->Fatigue01,
		Corrupt->Health01, Corrupt->TimeStarvingHours);
	TestEqual(TEXT("negative hunger clamped to 0"), Life.GetNeeds().GetHunger01(), 0.f);
	TestEqual(TEXT("excess fatigue clamped to 1"), Life.GetNeeds().GetFatigue01(), 1.f);
	TestEqual(TEXT("negative health clamped to 0"), Life.GetNeeds().GetHealth01(), 0.f);
	TestEqual(TEXT("negative starving time clamped to 0"), Life.GetNeeds().GetTimeStarving(), 0.f);

	Clock.Restore(Corrupt->DayIndex, Corrupt->HourOfDay);
	TestEqual(TEXT("negative day clamped to 1"), Clock.GetDayIndex(), 1);
	TestEqual(TEXT("excess hour wrapped to [0,24)"), Clock.GetHourOfDay(), FMath::Fmod(48.f, 24.f));

	FCourier404ContractService Contracts;
	Courier404SliceContracts::RegisterDefaults(Contracts);
	for (const FCourier404ContractRecord& R : Corrupt->ContractRecords)
	{
		FContractRuntimeState S;
		S.InstanceId = R.InstanceId;
		S.ContractId = R.ContractId;
		S.Status = R.Status;
		Contracts.RestoreInstance(S);
	}
	TestEqual(TEXT("empty contract records restore without crash"), Contracts.GetInstances().Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404PersistDeterministicRestore,
	"Courier404.Persistence.DeterministicRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404PersistDeterministicRestore::RunTest(const FString& Parameters)
{
	auto BuildState = [&](FCourier404SimClock& Clock, FCourier404LifeService& Life, FCourier404ContractService& Contracts)
	{
		Courier404SliceContracts::RegisterDefaults(Contracts);
		Life.GetWallet().Add(175);
		const FName Instance = Contracts.Accept(TEXT("Job.Courier01"), 5.f);
		Contracts.MarkPickup(Instance, 30.f);
		Life.Advance(6.f);
		Clock.AdvanceHours(19.f);
	};

	auto Capture = [&](FCourier404SimClock& Clock, FCourier404LifeService& Life, FCourier404ContractService& Contracts,
		UCourier404SaveGame* Save)
	{
		Save->Money = Life.GetWallet().GetBalance();
		Save->DayIndex = Clock.GetDayIndex();
		Save->HourOfDay = Clock.GetHourOfDay();
		Save->Hunger01 = Life.GetNeeds().GetHunger01();
		Save->Fatigue01 = Life.GetNeeds().GetFatigue01();
		Save->Health01 = Life.GetNeeds().GetHealth01();
		Save->TimeStarvingHours = Life.GetNeeds().GetTimeStarving();
		Save->ContractRecords.Reset();
		for (const TPair<FName, FContractRuntimeState>& Pair : Contracts.GetInstances())
		{
			FCourier404ContractRecord R;
			R.InstanceId = Pair.Value.InstanceId;
			R.ContractId = Pair.Value.ContractId;
			R.Status = Pair.Value.Status;
			R.AcceptedAtSimSeconds = Pair.Value.AcceptedAtSimSeconds;
			R.PickedUpAtSimSeconds = Pair.Value.PickedUpAtSimSeconds;
			R.FailureReason = Pair.Value.FailureReason;
			Save->ContractRecords.Add(R);
		}
	};

	auto Apply = [&](UCourier404SaveGame* Save, FCourier404SimClock& Clock, FCourier404LifeService& Life, FCourier404ContractService& Contracts)
	{
		Courier404SliceContracts::RegisterDefaults(Contracts);
		for (const FCourier404ContractRecord& R : Save->ContractRecords)
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
			Contracts.RestoreInstance(S);
		}
		Life.GetWallet().SetBalance(Save->Money);
		Clock.Restore(Save->DayIndex, Save->HourOfDay);
		Life.GetNeeds().Restore(Save->Hunger01, Save->Fatigue01, Save->Health01, Save->TimeStarvingHours);
	};

	FCourier404SimClock ClockA;
	FCourier404LifeService LifeA(&ClockA);
	FCourier404ContractService ContractsA;
	BuildState(ClockA, LifeA, ContractsA);

	UCourier404SaveGame* Save = NewObject<UCourier404SaveGame>();
	Capture(ClockA, LifeA, ContractsA, Save);

	FCourier404SimClock ClockB;
	FCourier404LifeService LifeB(&ClockB);
	FCourier404ContractService ContractsB;
	Apply(Save, ClockB, LifeB, ContractsB);

	FCourier404SimClock ClockC;
	FCourier404LifeService LifeC(&ClockC);
	FCourier404ContractService ContractsC;
	Apply(Save, ClockC, LifeC, ContractsC);

	TestEqual(TEXT("money identical across restores"), LifeB.GetWallet().GetBalance(), LifeC.GetWallet().GetBalance());
	TestEqual(TEXT("day identical across restores"), ClockB.GetDayIndex(), ClockC.GetDayIndex());
	TestEqual(TEXT("hour identical across restores"), ClockB.GetHourOfDay(), ClockC.GetHourOfDay());
	TestEqual(TEXT("hunger identical across restores"), LifeB.GetNeeds().GetHunger01(), LifeC.GetNeeds().GetHunger01());
	TestEqual(TEXT("fatigue identical across restores"), LifeB.GetNeeds().GetFatigue01(), LifeC.GetNeeds().GetFatigue01());
	TestEqual(TEXT("health identical across restores"), LifeB.GetNeeds().GetHealth01(), LifeC.GetNeeds().GetHealth01());
	TestEqual(TEXT("starving time identical across restores"), LifeB.GetNeeds().GetTimeStarving(), LifeC.GetNeeds().GetTimeStarving());
	TestEqual(TEXT("contract count identical across restores"), ContractsB.GetInstances().Num(), ContractsC.GetInstances().Num());

	int32 PayoutB = 0;
	int32 PayoutC = 0;
	for (const TPair<FName, FContractRuntimeState>& Pair : ContractsB.GetInstances())
	{
		if (Pair.Value.Status == EContractStatus::PickedUp)
		{
			int32 Payout = 0;
			ContractsB.TryDeliver(Pair.Key, TEXT("Point.DropA"), 1.f, Payout);
			PayoutB += Payout;
		}
	}
	for (const TPair<FName, FContractRuntimeState>& Pair : ContractsC.GetInstances())
	{
		if (Pair.Value.Status == EContractStatus::PickedUp)
		{
			int32 Payout = 0;
			ContractsC.TryDeliver(Pair.Key, TEXT("Point.DropA"), 1.f, Payout);
			PayoutC += Payout;
		}
	}
	TestEqual(TEXT("payout identical across restores"), PayoutB, PayoutC);

	return true;
}

#endif
