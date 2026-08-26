#pragma once

#include "Commandlets/Commandlet.h"
#include "BuildDistrictCommandlet.h"
#include "SliceLifecycleCommandlet.generated.h"

/**
 * Deterministic end-to-end proof of the vertical-slice checklist:
 * accept -> pickup -> deliver -> eat -> sleep -> anon night flow -> police
 * arrest -> hostile beating -> death recovery -> save/load stability.
 */
UCLASS()
class USliceLifecycleCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	USliceLifecycleCommandlet();
	virtual int32 Main(const FString& Params) override;
};
