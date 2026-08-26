#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/Courier404Interaction.h"
#include "Components/BoxComponent.h"

class FCourier404LifeService;

#include "Courier404Bed.generated.h"

/**
 * Apartment bed. Interacting sleeps until 07:00 next occurrence: advances the
 * simulated clock and restores fatigue.
 */
UCLASS()
class COURIER404_API ACourier404Bed : public AActor, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	ACourier404Bed();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Courier404")
	float WakeUpHour = 7.f;

	/** Test seam: when null resolves ULifeSubsystem from the GameInstance. */
	FCourier404LifeService* LifeOverride = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Mattress;

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;

private:
	FCourier404LifeService* ResolveLife() const;
};
