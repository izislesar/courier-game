#pragma once

#include "GameFramework/Actor.h"
#include "Police/PoliceEncounter.h"
#include "Components/BoxComponent.h"
#include "Courier404PoliceTrigger.generated.h"

class UBoxComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FCourierOnPoliceStopRequested, ECourier404StopReason);

/**
 * Patrol-zone volume: entering it requests a police stop. Handlers wire the
 * request into FCourier404PoliceEncounter via the life/contract services.
 */
UCLASS()
class COURIER404_API ACourier404PoliceTrigger : public AActor
{
	GENERATED_BODY()

public:
	ACourier404PoliceTrigger();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Zone;

	/** ECourier404StopReason as uint8 for reflection. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Courier404", meta = (Bitmask))
	uint8 Reason = static_cast<uint8>(ECourier404StopReason::TrafficViolation);

	FCourierOnPoliceStopRequested OnStopRequested;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
