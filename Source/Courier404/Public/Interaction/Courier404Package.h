#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/Courier404Interaction.h"
#include "Courier404Package.generated.h"

class UBoxComponent;
class FCourier404ContractService;
class UContractServiceSubsystem;

UENUM(BlueprintType)
enum class EPackageState : uint8
{
	Free,
	Carried,
	InVehicle,
	Delivered
};

/**
 * One physical parcel. Pickup/drop via the common interaction system; delivery
 * truth stays in the contract service — this actor only tracks physical state.
 */
UCLASS(Blueprintable)
class COURIER404_API ACourier404Package : public AActor, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	ACourier404Package();

	/** Cargo identity matched against the active contract definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Courier404")
	FName CargoId = TEXT("Cargo.Parcel");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UBoxComponent> BoxCollision;

	/** Test seam: when null the package resolves the GameInstance subsystem domain. */
	FCourier404ContractService* DomainOverride = nullptr;

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;

	EPackageState GetState() const { return State; }

	/** Player picks the package up (attaches to carrier component). */
	bool TryPickup(AActor* PlayerInteractor);

	/** Puts the package down at its current location. */
	bool Drop();

	/** Loads the package into a vehicle's cargo facade. */
	bool PlaceInVehicle(AActor* Vehicle);
	bool RetrieveFromVehicle(AActor* PlayerInteractor);

	/** Terminal state after validated delivery; removes the parcel from play. */
	void MarkDelivered();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	EPackageState State = EPackageState::Free;

	UPROPERTY()
	TWeakObjectPtr<AActor> Holder;

	FCourier404ContractService* ResolveContracts() const;
};
