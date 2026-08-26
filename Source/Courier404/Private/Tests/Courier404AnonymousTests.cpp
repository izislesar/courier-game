#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Contracts/SliceContracts.h"
#include "Contracts/ContractDomain.h"
#include "Time/SimClock.h"
#include "UI/PhoneViewModel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404AnonymousFlow,
	"Courier404.Anonymous.NightOfferHighPayoutStableCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404AnonymousFlow::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock; // day 1, 08:00
	FCourier404ContractService Service;
	Courier404SliceContracts::RegisterDefaults(Service);

	int32 PayoutEvents = 0;
	int32 LastPayout = 0;
	Service.OnContractCompleted.AddLambda([&](const FContractRuntimeState&, int32 Payout)
	{
		++PayoutEvents;
		LastPayout = Payout;
	});
	int32 FailEvents = 0;
	Service.OnContractFailed.AddLambda([&](const FContractRuntimeState&) { ++FailEvents; });

	// Phone presentation: hidden by day, visible at night, much higher payout.
	FCourier404PhoneViewModel VM;
	VM.Bind(&Service, &Clock);
	VM.Refresh();
	bool bAnonByDay = false;
	for (const FCourier404PhoneViewModel::FOffer& O : VM.GetOffers())
	{
		bAnonByDay |= O.Category == EContractCategory::Anonymous;
	}
	TestFalse(TEXT("no anon offer by day"), bAnonByDay);

	Clock.SleepTo(22.f); // evening
	VM.Refresh();
	const FCourier404PhoneViewModel::FOffer* AnonOffer = nullptr;
	for (const FCourier404PhoneViewModel::FOffer& O : VM.GetOffers())
	{
		if (O.Category == EContractCategory::Anonymous)
		{
			AnonOffer = &O;
		}
	}
	if (!TestNotNull(TEXT("anon offer at night"), const_cast<FCourier404PhoneViewModel::FOffer*>(AnonOffer)))
	{
		return false;
	}
	TestTrue(TEXT("pays much more than normal"), AnonOffer->Reward >= 5 * 120);

	// Reject path: stable, no instance created.
	const int32 OffersBefore = VM.GetOffers().Num();
	VM.MoveSelection(10); // select the anonymous (last) offer
	TestTrue(TEXT("reject works"), VM.RejectSelected());
	TestEqual(TEXT("offer list shrank"), VM.GetOffers().Num(), OffersBefore - 1);
	TestTrue(TEXT("no instance after reject"), Service.GetInstances().Num() == 0);

	// Re-offer still available next refresh (rejection is session-scoped per offer,
	// but a fresh VM sees it again): accept through service this time.
	const FName Instance = Service.Accept(TEXT("Job.NightDrop"), Clock.GetHourOfDay() * 3600.f);
	TestFalse(TEXT("accept creates active contract"), Instance.IsNone());

	// High-risk hook: sealed bag elevates police risk for carriers.
	TestEqual(TEXT("parcel has no police risk"), Service.GetPoliceRiskForCargo(TEXT("Cargo.Parcel")), 0);
	TestEqual(TEXT("sealed bag carries risk 3"), Service.GetPoliceRiskForCargo(TEXT("Cargo.SealedBag")), 3);

	// Complete through the SAME architecture as normal work: pickup then risky drop.
	const FName Cargo = TEXT("Cargo.SealedBag");
	TestEqual(TEXT("cargo binds to anon job"), Service.FindActiveInstanceForCargo(Cargo), Instance);
	TestTrue(TEXT("pickup marked"), Service.MarkPickup(Instance, Clock.GetHourOfDay() * 3600.f + 600.f));

	int32 Payout = 0;
	TestFalse(TEXT("wrong drop rejected"), Service.TryDeliver(Instance, TEXT("Point.DropA"), 1.f, Payout));
	TestTrue(TEXT("risky drop completes"), Service.TryDeliver(Instance, TEXT("Point.RiskyDrop"), 2.f, Payout));
	TestEqual(TEXT("high payout once"), LastPayout, 900);
	TestEqual(TEXT("exactly one completion"), PayoutEvents, 1);
	TestEqual(TEXT("zero failures"), FailEvents, 0);

	// Failure stability: failing a terminal job is refused.
	TestFalse(TEXT("cannot fail completed job"), Service.Fail(Instance, TEXT("Test.Late"), 3.f));
	TestEqual(TEXT("failure count unchanged"), FailEvents, 0);

	return true;
}

#endif
