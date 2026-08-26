#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/Courier404Interaction.h"
#include "Components/BoxComponent.h"

class FCourier404LifeService;

#include "Courier404Girlfriend.generated.h"

/** Lena: interactable only during a scheduled meeting window. */
UCLASS()
class COURIER404_API ACourier404Girlfriend : public AActor, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	ACourier404Girlfriend();

	/** Test seam: when null resolves ULifeSubsystem from the GameInstance. */
	FCourier404LifeService* LifeOverride = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Zone;

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;

private:
	FCourier404LifeService* ResolveLife() const;
};
