#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/Courier404Interaction.h"
#include "Components/BoxComponent.h"
class FCourier404LifeService;
#include "Courier404FoodSource.generated.h"

/**
 * Store food counter. Interacting buys+eats one meal through the life service.
 */
UCLASS()
class COURIER404_API ACourier404FoodSource : public AActor, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	ACourier404FoodSource();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Courier404")
	int32 MealPrice = 15;

	/** Test seam: when null resolves ULifeSubsystem from the GameInstance. */
	FCourier404LifeService* LifeOverride = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Counter;

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;

private:
	FCourier404LifeService* ResolveLife() const;
};
