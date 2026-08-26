#pragma once

#include "Commandlets/Commandlet.h"
#include "GenerateInputAssetsCommandlet.generated.h"

/**
 * Headless generator for Enhanced Input assets (/Game/Input).
 * Idempotent: recreates IA_* and IMC_CourierDefault deterministically.
 */
UCLASS()
class UGenerateInputAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UGenerateInputAssetsCommandlet();
	virtual int32 Main(const FString& Params) override;
};
