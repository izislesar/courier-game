#pragma once

#include "GameFramework/Pawn.h"
#include "Interaction/Courier404Interaction.h"
#include "Courier404VehiclePawn.generated.h"

class UBoxComponent;
class UVehicleFacadeComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * One ordinary used car. Lite-physics drivetrain for pre-prod (placeholder
 * assets, headless-testable); the game-facing API matches the Chaos upgrade
 * path so internals can be swapped without touching gameplay code.
 */
UCLASS()
class COURIER404_API ACourier404VehiclePawn : public APawn, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	ACourier404VehiclePawn();

	// ~ICourier404Interactable: interacting enters/exits the vehicle.
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;

	/** Enters with possession swap; remembers the driver's pawn for exit. */
	bool TryEnterVehicle(APawn* DriverPawn);

	/** Exits, restoring the remembered pawn beside the vehicle. */
	bool TryExitVehicle();

	bool IsOccupied() const;
	float GetSpeedKmh() const;

	/** True while the player is driving (possession held by this pawn). */
	bool IsBeingDriven() const { return DriverPawn.IsValid(); }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UBoxComponent> BodyCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UVehicleFacadeComponent> Facade;

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	void ApplyDrive(const FInputActionValue& Value);
	void ApplyBrake();
	void ReleaseBrake();
	void RequestExit();
	void UpdateFuel(float DeltaSeconds, float ThrottleMagnitude);

	// Drivetrain tuning (arcade-lite; readable over simulated).
	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Drive")
	float MaxSpeedKmh = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Drive")
	float MaxReverseSpeedKmh = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Drive")
	float Acceleration = 450.f;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Drive")
	float BrakeDeceleration = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Drive")
	float SteerRateDegPerSec = 70.f;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> DriveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> InteractExitAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TSoftObjectPtr<UInputMappingContext> DriveMappingContext;

private:
	void AddMappingContext();

	UPROPERTY()
	TWeakObjectPtr<APawn> DriverPawn;

	UPROPERTY()
	TObjectPtr<USceneComponent> TrunkSocket;

	float ForwardSpeed = 0.f; // cm/s signed
	float SteerInput = 0.f;
	float ThrottleInput = 0.f;
	bool bHandbrake = false;
};
