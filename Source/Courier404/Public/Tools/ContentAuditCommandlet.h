#pragma once

#include "Commandlets/Commandlet.h"
#include "ContentAuditCommandlet.generated.h"

/**
 * Automated content audit commandlet verifying:
 *   (1) texture dimensions comply with performance-budget.md,
 *   (2) meshes have valid LOD behavior,
 *   (3) materials use shared master materials,
 *   (4) no unused assets in package.
 * Runs in -nullrhi mode. Exits 0 when clear.
 */
UCLASS()
class UContentAuditCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UContentAuditCommandlet();
	virtual int32 Main(const FString& Params) override;
};
