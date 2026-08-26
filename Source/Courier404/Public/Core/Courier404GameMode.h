#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Courier404GameMode.generated.h"

/**
 * Minimal game mode proving the build/codegen pipeline.
 * Gameplay rules live in domain systems/subsystems, not here.
 */
UCLASS(Config = Game)
class COURIER404_API ACourier404GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACourier404GameMode();
};
