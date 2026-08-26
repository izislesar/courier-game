#pragma once

#include "Commandlets/Commandlet.h"
#include "BuildDistrictCommandlet.h"
#include "VerifyDistrictCommandlet.generated.h"

/** Loads /Game/Maps/District01 and asserts every vertical-slice landmark exists. */
UCLASS()
class UVerifyDistrictCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UVerifyDistrictCommandlet();
	virtual int32 Main(const FString& Params) override;
};
