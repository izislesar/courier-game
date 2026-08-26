#pragma once

#include "Commandlets/Commandlet.h"
#include "GenerateInputAssetsCommandlet.h"
#include "BuildDistrictCommandlet.generated.h"

/**
 * Builds /Game/Maps/District01: compact district blockout with all vertical-slice
 * landmarks, stable actor tags (Courier404Locations) and the baseline lighting rig.
 * Deterministic and idempotent.
 */
UCLASS()
class UBuildDistrictCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UBuildDistrictCommandlet();
	virtual int32 Main(const FString& Params) override;
};
