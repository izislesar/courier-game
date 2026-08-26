#include "Courier404VehicleTests.h"
#include "Misc/AutomationTest.h"
#include "Vehicle/VehicleFacade.h"
#include "Vehicle/Courier404VehiclePawn.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404VehicleFacadeTest,
	"Courier404.Vehicle.Facade",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404VehicleFacadeTest::RunTest(const FString& Parameters)
{
	UVehicleFacadeComponent* Facade = NewObject<UVehicleFacadeComponent>();
	AActor* DriverA = NewObject<AActor>();
	AActor* Package = NewObject<AActor>();

	// Occupancy state machine.
	TestTrue(TEXT("can enter when free"), Facade->CanEnter(DriverA));
	TestTrue(TEXT("enter succeeds"), Facade->Enter(DriverA));
	TestFalse(TEXT("cannot enter twice"), Facade->CanEnter(DriverA));
	TestFalse(TEXT("second enter refused"), Facade->Enter(NewObject<AActor>()));
	TestTrue(TEXT("driver tracked"), Facade->GetDriver() == DriverA);
	TestFalse(TEXT("wrong driver cannot exit"), Facade->Exit(NewObject<AActor>()));
	TestTrue(TEXT("correct driver exits"), Facade->Exit(DriverA));
	TestTrue(TEXT("seat free again"), Facade->CanEnter(DriverA));

	// Cargo slot: single package, retrieval clears.
	TestFalse(TEXT("no cargo initially"), Facade->HasCargo());
	TestTrue(TEXT("attach cargo"), Facade->AttachCargo(Package));
	TestFalse(TEXT("second package refused while loaded"), Facade->AttachCargo(NewObject<AActor>()));
	AActor* Retrieved = nullptr;
	TestTrue(TEXT("detach cargo"), Facade->DetachCargo(&Retrieved));
	TestEqual(TEXT("retrieved same package"), Retrieved, Package);
	TestFalse(TEXT("detach empty refused"), Facade->DetachCargo(nullptr));

	// Headlights + fuel + impacts.
	Facade->SetHeadlights(true);
	TestTrue(TEXT("headlights on"), Facade->AreHeadlightsOn());
	Facade->SetFuel(10.f);
	TestEqual(TEXT("consume within stock"), Facade->ConsumeFuel(4.f), 4.f);
	TestEqual(TEXT("consume clamps to remaining"), Facade->ConsumeFuel(99.f), 6.f);
	Facade->NotifyImpact(120.f, nullptr);
	Facade->NotifyImpact(20.f, nullptr); // below threshold: ignored
	TestEqual(TEXT("only significant impacts counted"), Facade->GetImpactCount(), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404VehiclePawnSmoke,
	"Courier404.Vehicle.PawnSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404VehiclePawnSmoke::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("world created"), World))
	{
		return false;
	}

	FURL Url;
	World->InitializeActorsForPlay(Url);
	World->BeginPlay();

	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;

	ACourier404VehiclePawn* Vehicle = World->SpawnActor<ACourier404VehiclePawn>(
		ACourier404VehiclePawn::StaticClass(), FVector(500.f, 0.f, 100.f), FRotator::ZeroRotator, Params);
	AMockDriverPawn* Driver = World->SpawnActor<AMockDriverPawn>(
		AMockDriverPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (!TestNotNull(TEXT("vehicle spawned"), Vehicle) || !TestNotNull(TEXT("driver spawned"), Driver))
	{
		World->DestroyWorld(false);
		return false;
	}

	// Idle vehicle reads as stationary and enterable.
	TestEqual(TEXT("speed zero at rest"), Vehicle->GetSpeedKmh(), 0.f);
	TestTrue(TEXT("interactable when free"), Vehicle->CanInteract(Driver));
	TestTrue(TEXT("prompt present"), !Vehicle->GetInteractionPrompt(Driver).IsEmpty());

	// Occupancy without possession (controller-less smoke): facade-level only.
	TestTrue(TEXT("facade accepts driver"), Vehicle->Facade->Enter(Driver));
	TestTrue(TEXT("occupied blocks interaction"), !Vehicle->CanInteract(Driver));
	TestTrue(TEXT("exit frees seat"), Vehicle->Facade->Exit(Driver));

	// Cargo flows through the pawn's facade.
	AActor* Package = NewObject<AActor>();
	TestTrue(TEXT("cargo attachable"), Vehicle->Facade->AttachCargo(Package));
	TestTrue(TEXT("vehicle carries cargo"), Vehicle->Facade->HasCargo());
	AActor* Out = nullptr;
	TestTrue(TEXT("cargo retrievable"), Vehicle->Facade->DetachCargo(&Out));
	TestEqual(TEXT("same package returned"), Out, Package);

	World->DestroyWorld(false);
	return true;
}

#endif
