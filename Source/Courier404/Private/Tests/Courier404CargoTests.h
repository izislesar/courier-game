#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/CargoCarrierComponent.h"
#include "Courier404CargoTests.generated.h"

/** Carrier pawn used in cargo flow tests. */
UCLASS()
class AMockCarrierPawn : public APawn
{
	GENERATED_BODY()

public:
	AMockCarrierPawn()
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
		Carrier = CreateDefaultSubobject<UCargoCarrierComponent>(TEXT("Carrier"));
	}

	UPROPERTY()
	TObjectPtr<UCargoCarrierComponent> Carrier;
};
