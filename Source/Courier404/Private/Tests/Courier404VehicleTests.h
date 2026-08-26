#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Courier404VehicleTests.generated.h"

/** Minimal driver pawn for occupancy tests (no movement component). */
UCLASS()
class AMockDriverPawn : public APawn
{
	GENERATED_BODY()

public:
	AMockDriverPawn()
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	}
};
