#pragma once

#include "Components/ActorComponent.h"
#include "CargoCarrierComponent.generated.h"

class ACourier404Package;

/**
 * Player-local cargo holder: exactly one carried package at a time.
 */
UCLASS(ClassGroup = (Courier404), meta = (BlueprintSpawnableComponent))
class COURIER404_API UCargoCarrierComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool Hold(ACourier404Package* Package);
	ACourier404Package* Release();
	ACourier404Package* GetHeld() const { return Held.Get(); }

private:
	UPROPERTY()
	TWeakObjectPtr<ACourier404Package> Held;
};
