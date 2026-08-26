#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/Courier404Interaction.h"
#include "Courier404DropPoint.generated.h"

class UBoxComponent;
class FCourier404ContractService;
class UContractServiceSubsystem;

/**
 * World delivery point. Validates the player's carried package against the
 * active contract; payout truth stays in the contract service.
 */
UCLASS(Blueprintable)
class COURIER404_API ACourier404DropPoint : public AActor, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	ACourier404DropPoint();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Courier404")
	FName DropPointId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UBoxComponent> Zone;

	/** Test seam mirroring the package actor. */
	FCourier404ContractService* DomainOverride = nullptr;

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;

private:
	FCourier404ContractService* ResolveContracts() const;
};
