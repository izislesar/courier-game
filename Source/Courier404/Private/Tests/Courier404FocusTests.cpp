#include "Misc/AutomationTest.h"
#include "Courier404FocusTests.h"
#include "Courier404.h"
#include "Interaction/InteractorComponent.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404FocusTest,
	"Courier404.Interaction.FocusTriggersTwoTypes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404FocusTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("world created"), World))
	{
		return false;
	}

	FURL Url;
	World->InitializeActorsForPlay(Url);
	World->BeginPlay();

	// Two different interactable implementations along the view ray (+X),
	// at camera height, inside interaction reach (250).
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;

	AMockComponentInteractable* CompActor = World->SpawnActor<AMockComponentInteractable>(
		AMockComponentInteractable::StaticClass(), FVector(150.f, 0.f, 70.f), FRotator::ZeroRotator, Params);
	AMockNativeInteractable* NativeActor = World->SpawnActor<AMockNativeInteractable>(
		AMockNativeInteractable::StaticClass(), FVector(220.f, 0.f, 70.f), FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("component actor"), CompActor) ||
		!TestNotNull(TEXT("native actor"), NativeActor))
	{
		return false;
	}

	UE_LOG(LogCourier404, Log, TEXT("TEST comp=%s native=%s"),
		*CompActor->GetActorLocation().ToString(), *NativeActor->GetActorLocation().ToString());

	AMockViewPawn* Player = World->SpawnActor<AMockViewPawn>(
		AMockViewPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("player spawned"), Player))
	{
		return false;
	}

	UInteractorComponent* Interactor = NewObject<UInteractorComponent>(Player);
	Interactor->RegisterComponent();

	// Diagnostic: raw visibility trace along the expected view ray.
	const UCameraComponent* Cam = Player->Camera;
	UE_LOG(LogCourier404, Log, TEXT("TEST cam loc=%s fwd=%s"),
		*Cam->GetComponentLocation().ToString(), *Cam->GetForwardVector().ToString());
	{
		FCollisionQueryParams DP(SCENE_QUERY_STAT(Diag), false, Player);
		FHitResult DH;
		const bool bHit = World->LineTraceSingleByChannel(DH, FVector(0, 0, 70), FVector(400, 0, 70), ECC_Visibility, DP);
		UE_LOG(LogCourier404, Log, TEXT("TEST diag hit=%d actor=%s"), bHit,
			bHit && DH.GetActor() ? *DH.GetActor()->GetName() : TEXT("none"));
	}

	// Focus resolves to the nearest interactable (component actor).
	Interactor->UpdateFocus();
	TestTrue(TEXT("focuses component actor"), Interactor->GetFocusedActor() == CompActor);
	TestTrue(TEXT("interact fires component type"), Interactor->TryInteract());

	// Disable nearest; focus falls through to the native-interface actor.
	CompActor->Interaction->bInteractionEnabled = false;
	Interactor->UpdateFocus();
	TestTrue(TEXT("focus falls to native actor"), Interactor->GetFocusedActor() == NativeActor);
	TestTrue(TEXT("interact fires native type"), Interactor->TryInteract());
	TestEqual(TEXT("native actor received exactly one interaction"), NativeActor->InteractCount, 1);

	// Nothing reachable -> no interaction.
	NativeActor->bEnabled = false;
	Interactor->UpdateFocus();
	TestFalse(TEXT("no interaction when all disabled"), Interactor->TryInteract());

	World->DestroyWorld(false);
	return true;
}

#endif
