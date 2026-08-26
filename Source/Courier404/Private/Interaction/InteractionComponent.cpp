#include "Interaction/InteractionComponent.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInteractionComponent::CanInteract(AActor* Interactor) const
{
	return bInteractionEnabled && IsValid(Interactor);
}

FText UInteractionComponent::GetInteractionPrompt(AActor* Interactor) const
{
	return CanInteract(Interactor) ? PromptText : FText();
}

void UInteractionComponent::Interact(AActor* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return;
	}

	OnInteracted.Broadcast(Interactor);
}
