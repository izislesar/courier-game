#pragma once

#include "Components/ActorComponent.h"
#include "VehicleFacade.generated.h"

/**
 * Game-facing vehicle state: occupancy, cargo slot, headlights, fuel, impact log.
 * Deliberately free of movement/physics internals so the drivetrain implementation
 * (lite physics now, Chaos later) stays swappable.
 */
UCLASS(ClassGroup = (Courier404), meta = (BlueprintSpawnableComponent))
class COURIER404_API UVehicleFacadeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehicleFacadeComponent();

	// -- Occupancy --
	bool CanEnter(AActor* CandidateDriver) const;
	bool Enter(AActor* Driver);
	bool Exit(AActor* Driver);
	AActor* GetDriver() const { return Driver.Get(); }

	// -- Cargo --
	/** Single package slot for the slice. Attaches CargoActor to the trunk socket owner. */
	bool AttachCargo(AActor* Package);
	bool DetachCargo(AActor** OutPackage = nullptr);
	bool HasCargo() const { return IsValid(CargoPackage.Get()); }
	AActor* GetCargo() const { return CargoPackage.Get(); }

	// -- Headlights --
	void SetHeadlights(bool bOn) { bHeadlightsOn = bOn; }
	bool AreHeadlightsOn() const { return bHeadlightsOn; }

	// -- Fuel (optional slice feature; simple scalar) --
	void SetFuel(float Liters) { FuelLiters = FMath::Max(Liters, 0.f); }
	float ConsumeFuel(float Liters);
	float GetFuel() const { return FuelLiters; }

	// -- Collision consequence --
	void NotifyImpact(float RelativeSpeed, AActor* Other);
	int32 GetImpactCount() const { return ImpactCount; }

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> Driver;

	UPROPERTY()
	TWeakObjectPtr<AActor> CargoPackage;

	bool bHeadlightsOn = false;
	float FuelLiters = 40.f;
	int32 ImpactCount = 0;
};
