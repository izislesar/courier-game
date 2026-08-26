#pragma once

#include "Components/ActorComponent.h"
#include "Interaction/Courier404Interaction.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteracted, AActor*, Interactor);

/**
 * Actor-local interaction behavior. Attach to any interactable actor and
 * optionally delegate to an owner-provided handler via OnInteracted.
 */
UCLASS(ClassGroup = (Courier404), meta = (BlueprintSpawnableComponent))
class COURIER404_API UInteractionComponent : public UActorComponent, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Courier404|Interaction")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Courier404|Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(BlueprintAssignable, Category = "Courier404|Interaction")
	FOnInteracted OnInteracted;

	// ~ICourier404Interactable
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;
};
