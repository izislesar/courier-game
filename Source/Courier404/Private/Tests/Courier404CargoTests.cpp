#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Courier404.h"
#include "Contracts/ContractDomain.h"
#include "Interaction/Courier404Package.h"
#include "Interaction/Courier404DropPoint.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404CargoPickupGating,
	"Courier404.Cargo.PickupRequiresActiveContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404CargoPickupGating::RunTest(const FString& Parameters)
{
	// This suite exercises the service-level gating directly because the package's
	// contract resolution is the same FindActiveInstanceForCargo call.
	FCourier404ContractService Service;
	FContractDefinition Def;
	Def.ContractId = TEXT("Job.CargoA");
	Def.PickupPointId = TEXT("Point.Store");
	Def.DropoffPointId = TEXT("Point.DropA");
	Def.Reward = 100;
	Def.CargoId = TEXT("Cargo.Parcel");
	TArray<FString> Errors;
	TestTrue(TEXT("registered"), Service.RegisterDefinition(Def, Errors));

	// No active job: gate refuses.
	TestTrue(TEXT("gate closed without contract"), Service.FindActiveInstanceForCargo(TEXT("Cargo.Parcel")).IsNone());

	const FName Instance = Service.Accept(TEXT("Job.CargoA"), 0.f);
	TestEqual(TEXT("gate opens after accept"), Service.FindActiveInstanceForCargo(TEXT("Cargo.Parcel")), Instance);

	// Exactly once: after pickup the Accepted gate closes and PickedUp opens.
	Service.MarkPickup(Instance, 1.f);
	TestTrue(TEXT("accepted gate closed after pickup"), Service.FindActiveInstanceForCargo(TEXT("Cargo.Parcel")).IsNone());
	TestEqual(TEXT("picked-up gate open"), Service.FindPickedUpInstanceForCargo(TEXT("Cargo.Parcel")), Instance);

	// A second accepted job reopens the gate (multiple packages supported).
	const FName Second = Service.Accept(TEXT("Job.CargoA"), 5.f);
	TestEqual(TEXT("oldest-first ordering"), Service.FindPickedUpInstanceForCargo(TEXT("Cargo.Parcel")), Instance);
	TestTrue(TEXT("second instance active"), !Second.IsNone());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404CargoDeliveryFlow,
	"Courier404.Cargo.DeliveryOnlyAtCorrectDrop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404CargoDeliveryFlow::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("world"), World))
	{
		return false;
	}
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	FCourier404ContractService Service;

	FContractDefinition Def;
	Def.ContractId = TEXT("Job.CargoA");
	Def.PickupPointId = TEXT("Point.Store");
	Def.DropoffPointId = TEXT("Point.DropA");
	Def.Reward = 100;
	Def.TimeLimitSeconds = 600.f;
	Def.CargoId = TEXT("Cargo.Parcel");
	TArray<FString> Errors;
	Service.RegisterDefinition(Def, Errors);

	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	AMockCarrierPawn* Player = World->SpawnActor<AMockCarrierPawn>(
		AMockCarrierPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	ACourier404Package* Package = World->SpawnActor<ACourier404Package>(
		ACourier404Package::StaticClass(), FVector(200.f, 0.f, 0.f), FRotator::ZeroRotator, Params);
	ACourier404DropPoint* RightDrop = World->SpawnActor<ACourier404DropPoint>(
		ACourier404DropPoint::StaticClass(), FVector(400.f, 0.f, 0.f), FRotator::ZeroRotator, Params);
	RightDrop->DropPointId = TEXT("Point.DropA");
	RightDrop->DomainOverride = &Service;
	ACourier404DropPoint* WrongDrop = World->SpawnActor<ACourier404DropPoint>(
		ACourier404DropPoint::StaticClass(), FVector(-400.f, 0.f, 0.f), FRotator::ZeroRotator, Params);
	WrongDrop->DropPointId = TEXT("Point.Wrong");
	WrongDrop->DomainOverride = &Service;

	Package->DomainOverride = &Service;

	if (!TestNotNull(TEXT("player"), Player) || !TestNotNull(TEXT("package"), Package))
	{
		return false;
	}

	int32 CompletionCount = 0;
	int32 LastPayout = 0;
	Service.OnContractCompleted.AddLambda([&](const FContractRuntimeState&, int32 Payout)
	{
		++CompletionCount;
		LastPayout = Payout;
	});

	const FName Instance = Service.Accept(TEXT("Job.CargoA"), 0.f);
	UE_LOG(LogCourier404, Log, TEXT("CARGO accepted id=%s"), *Instance.ToString());
	TestFalse(TEXT("instance ready"), Instance.IsNone());

	auto StatusOf = [&](FName Id) -> EContractStatus
	{
		const FContractRuntimeState* S = Service.FindInstance(Id);
		return S ? S->Status : EContractStatus::Available;
	};

	// Pickup through the interaction API.
	Package->Interact(Player);
	TestEqual(TEXT("package carried"), Package->GetState(), EPackageState::Carried);
	TestEqual(TEXT("contract picked up"),
		StatusOf(Instance), EContractStatus::PickedUp);

	// Interacting again puts it down; pick it back up for delivery attempt.
	Package->Interact(Player);
	TestEqual(TEXT("put down returns to free"), Package->GetState(), EPackageState::Free);
	Package->Interact(Player);
	TestEqual(TEXT("carried again"), Package->GetState(), EPackageState::Carried);

	// Wrong drop point: nothing happens.
	WrongDrop->Interact(Player);
	TestEqual(TEXT("wrong drop keeps picked up"),
		StatusOf(Instance), EContractStatus::PickedUp);
	TestEqual(TEXT("no payout at wrong drop"), CompletionCount, 0);

	// Correct drop point: completes exactly once, consumes package.
	RightDrop->Interact(Player);
	TestEqual(TEXT("completed"), StatusOf(Instance), EContractStatus::Completed);
	TestEqual(TEXT("payout fired once"), CompletionCount, 1);
	TestEqual(TEXT("payout amount"), LastPayout, 100);
	TestEqual(TEXT("package delivered"), Package->GetState(), EPackageState::Delivered);
	TestFalse(TEXT("carrier emptied"), Player->Carrier->GetHeld() != nullptr);

	// Delivered package cannot be reused.
	TestFalse(TEXT("delivered not interactable"), Package->CanInteract(Player));

	World->DestroyWorld(false);
	return true;
}

#endif
