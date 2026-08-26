#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/LifeService.h"
#include "LifeSubsystem.generated.h"

/**
 * Game-facing owner of the player life service (wallet, needs, rest).
 * World interactables resolve this via the GameInstance.
 */
UCLASS()
class COURIER404_API ULifeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FCourier404LifeService& GetLife() { return Life; }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	FCourier404SimClock Clock;
	FCourier404LifeService Life;
};
