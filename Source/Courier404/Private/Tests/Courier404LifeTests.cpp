#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Economy/Wallet.h"
#include "Core/LifeService.h"
#include "Time/SimClock.h"
#include "Contracts/SliceContracts.h"
#include "Contracts/ContractDomain.h"
#include "Needs/NeedsModel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404DeliveryPaysWallet,
	"Courier404.Life.DeliveryPayoutCreditsWalletOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404DeliveryPaysWallet::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock;
	FCourier404LifeService Life(&Clock);
	Life.GetWallet().Add(50); // starting cash

	FCourier404ContractService Contracts;
	Courier404SliceContracts::RegisterDefaults(Contracts);

	// Economy bridge mirrors ULifeSubsystem wiring.
	int32 PayoutEvents = 0;
	Contracts.OnContractCompleted.AddLambda([&](const FContractRuntimeState&, int32 Payout)
	{
		Life.GetWallet().Add(Payout);
		++PayoutEvents;
	});

	const FName Instance = Contracts.Accept(TEXT("Job.Courier01"), 0.f);
	TestFalse(TEXT("slice job accepted"), Instance.IsNone());

	const FName Cargo = TEXT("Cargo.Parcel");
	const FName PickupInstance = Contracts.FindActiveInstanceForCargo(Cargo);
	TestEqual(TEXT("cargo binds to accepted job"), PickupInstance, Instance);

	Contracts.MarkPickup(PickupInstance, 30.f);
	int32 Payout = 0;
	TestTrue(TEXT("delivery completes"), Contracts.TryDeliver(PickupInstance, TEXT("Point.DropA"), 60.f, Payout));
	TestEqual(TEXT("payout amount"), Payout, 120);
	TestEqual(TEXT("wallet credited"), Life.GetWallet().GetBalance(), 170);
	TestEqual(TEXT("single completion event"), PayoutEvents, 1);

	// Duplicate completion cannot pay twice.
	int32 Second = 0;
	TestFalse(TEXT("duplicate delivery refused"), Contracts.TryDeliver(PickupInstance, TEXT("Point.DropA"), 61.f, Second));
	TestEqual(TEXT("wallet unchanged"), Life.GetWallet().GetBalance(), 170);
	TestEqual(TEXT("still one event"), PayoutEvents, 1);

	return true;
}



IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404WalletGuards,
	"Courier404.Life.WalletNeverInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404WalletGuards::RunTest(const FString& Parameters)
{
	FCourier404Wallet Wallet;
	TestEqual(TEXT("starts at zero"), Wallet.GetBalance(), 0);

	Wallet.Add(-50); // negative adds ignored
	TestEqual(TEXT("negative add ignored"), Wallet.GetBalance(), 0);

	Wallet.Add(200);
	TestEqual(TEXT("income lands"), Wallet.GetBalance(), 200);

	TestFalse(TEXT("cannot overspend"), Wallet.TrySpend(201));
	TestEqual(TEXT("overspend left balance intact"), Wallet.GetBalance(), 200);

	TestTrue(TEXT("valid spend works"), Wallet.TrySpend(150));
	TestEqual(TEXT("balance reduced"), Wallet.GetBalance(), 50);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404LifeEat,
	"Courier404.Life.EatUpdatesNeedsAndMoneyAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404LifeEat::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock;
	FCourier404LifeService Life(&Clock);
	Life.GetWallet().Add(100);

	TestTrue(TEXT("affordable meal succeeds"), Life.Eat(/*elapsed*/ 4.f, /*price*/ 15));
	TestEqual(TEXT("charged exactly the price"), Life.GetWallet().GetBalance(), 85);
	TestTrue(TEXT("hunger restored to fed"),
		Life.GetNeeds().GetHunger() == ECourier404Hunger::Fed || Life.GetNeeds().GetHunger() == ECourier404Hunger::Hungry
			? Life.GetNeeds().GetHunger01() >= 0.7f
			: false);

	// Unaffordable: nothing changes at all.
	FCourier404SimClock ClockB;
	FCourier404LifeService Broke(&ClockB);
	Broke.GetWallet().Add(5);
	Broke.Advance(10.f);
	const float PreHunger = Broke.GetNeeds().GetHunger01();
	const int32 PreBalance = Broke.GetWallet().GetBalance();
	TestFalse(TEXT("poor customer refused"), Broke.Eat(0.f, 15));
	TestEqual(TEXT("refusal keeps wallet"), Broke.GetWallet().GetBalance(), PreBalance);
	TestEqual(TEXT("refusal keeps hunger"), Broke.GetNeeds().GetHunger01(), PreHunger);

	// Invalid prices refused even when rich.
	FCourier404SimClock ClockC;
	FCourier404LifeService Rich(&ClockC);
	Rich.GetWallet().Add(1000);
	TestFalse(TEXT("zero price refused"), Rich.Eat(0.f, 0));
	TestFalse(TEXT("negative price refused"), Rich.Eat(0.f, -3));
	TestEqual(TEXT("rich wallet untouched"), Rich.GetWallet().GetBalance(), 1000);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404LifeSleep,
	"Courier404.Life.BedSleepAdvancesClockAndRestores",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404LifeSleep::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock; // day 1, 08:00
	Clock.AdvanceHours(15.f);  // 23:00

	FCourier404LifeService Life(&Clock);
	Life.Advance(9.f); // awake through midnight: needs degraded, ~Tired/Exhausted edge
	TestFalse(TEXT("pre-sleep not fully rested"),
		Life.GetNeeds().GetFatigue() == ECourier404Fatigue::Rested && Life.GetNeeds().GetFatigue01() > 0.95f);

	const float Slept = Life.SleepTo(7.f); // next 07:00 is day 2
	TestEqual(TEXT("slept eight hours"), Slept, 8.f);
	TestEqual(TEXT("clock rolled to day 2"), Clock.GetDayIndex(), 2);
	TestEqual(TEXT("woke at seven"), Clock.GetHourOfDay(), 7.f);
	TestTrue(TEXT("rested after full night"), Life.GetNeeds().GetFatigue01() >= 0.6f);

	// Sleeping again immediately is a no-op jump.
	FCourier404SimClock ClockC;
	ClockC.AdvanceHours(-1.f); // no-op on clock
	FCourier404LifeService Fresh(&ClockC);
	TestEqual(TEXT("sleeping to current hour does nothing"), Fresh.SleepTo(8.f), 0.f);
	TestEqual(TEXT("day unchanged"), ClockC.GetDayIndex(), 1);

	return true;
}

#endif
