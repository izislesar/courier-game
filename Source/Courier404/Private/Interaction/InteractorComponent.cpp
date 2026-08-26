#include "Interaction/InteractorComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
}

void UInteractorComponent::UpdateFocus()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	const APawn* Pawn = Cast<APawn>(Owner);
	const UCameraComponent* Camera = Pawn ? Pawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!Camera)
	{
		return;
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * InteractionDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(Courier404Interact), false, Owner);

	// Walk the view ray front-to-back; focus the first actor that accepts interaction
	// right now. Blocked/disabled interactables are ignored and the trace continues
	// past them so players can reach interactables behind obstructions.
	AActor* NewFocus = nullptr;
	for (int32 Step = 0; Step < 8 && !NewFocus; ++Step)
	{
		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			break;
		}

		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor))
		{
			break;
		}

		if (UInteractionComponent* Component = HitActor->FindComponentByClass<UInteractionComponent>())
		{
			if (Component->CanInteract(Owner))
			{
				NewFocus = HitActor;
			}
		}
		else if (ICourier404Interactable* Interactable = Cast<ICourier404Interactable>(HitActor))
		{
			if (Interactable->CanInteract(Owner))
			{
				NewFocus = HitActor;
			}
		}

		if (!NewFocus)
		{
			// Not interactable right now: look past it.
			Params.AddIgnoredActor(HitActor);
		}
	}

	SetFocusedActor(NewFocus);
}

bool UInteractorComponent::TryInteract()
{
	AActor* Focus = FocusedActor.Get();
	if (!Focus)
	{
		return false;
	}

	AActor* Owner = GetOwner();

	if (UInteractionComponent* Component = Focus->FindComponentByClass<UInteractionComponent>())
	{
		if (Component->CanInteract(Owner))
		{
			Component->Interact(Owner);
			return true;
		}
		return false;
	}

	if (ICourier404Interactable* Interactable = Cast<ICourier404Interactable>(Focus))
	{
		if (Interactable->CanInteract(Owner))
		{
			Interactable->Interact(Owner);
			return true;
		}
	}

	return false;
}

void UInteractorComponent::SetFocusedActor(AActor* NewFocus)
{
	AActor* Old = FocusedActor.Get();
	if (Old == NewFocus)
	{
		return;
	}

	FocusedActor = NewFocus;

	FText Prompt;
	if (!IsValid(NewFocus))
	{
		OnFocusChanged.Broadcast(nullptr, Prompt);
		return;
	}
	if (UInteractionComponent* Component = NewFocus->FindComponentByClass<UInteractionComponent>())
	{
		Prompt = Component->GetInteractionPrompt(GetOwner());
	}
	else if (ICourier404Interactable* Interactable = Cast<ICourier404Interactable>(NewFocus))
	{
		Prompt = Interactable->GetInteractionPrompt(GetOwner());
	}

	OnFocusChanged.Broadcast(NewFocus, Prompt);
}
