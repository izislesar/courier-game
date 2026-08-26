#pragma once

#include "CoreMinimal.h"
#include "Contracts/Courier404Contracts.h"
#include "Contracts/ContractDomain.h"

/**
 * The two slice contracts (ordinary + anonymous), registered deterministically
 * at service init. Data-asset authoring can replace this later without changing
 * the runtime model.
 */
namespace Courier404SliceContracts
{
	inline constexpr auto NormalId = TEXT("Job.Courier01");
	inline constexpr auto AnonymousId = TEXT("Job.NightDrop");

	COURIER404_API void RegisterDefaults(FCourier404ContractService& Service);
}
