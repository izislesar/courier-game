#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Courier404Interaction.generated.h"

UINTERFACE(MinimalAPI)
class UCourier404Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Reusable native interaction contract implemented by world actors/components.
 * The player interactor queries this interface; concrete behavior stays actor-local.
 * C++-only by design (core interaction routing never lives in Blueprints).
 */
class COURIER404_API ICourier404Interactable
{
	GENERATED_BODY()

public:
	virtual bool CanInteract(AActor* Interactor) const { return true; }
	virtual FText GetInteractionPrompt(AActor* Interactor) const { return FText(); }
	virtual void Interact(AActor* Interactor) {}
};
