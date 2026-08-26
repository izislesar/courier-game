#pragma once

#include "CoreMinimal.h"

/**
 * Stable location identifiers shared by content and gameplay systems.
 * Never hard-code these strings outside this file and the district builder.
 */
namespace Courier404Locations
{
	inline constexpr auto Apartment   = TEXT("Loc.Apartment");
	inline constexpr auto Parking     = TEXT("Loc.Parking");
	inline constexpr auto Store       = TEXT("Loc.Store");
	inline constexpr auto PickupOrdinary = TEXT("Loc.PickupA");
	inline constexpr auto DropOrdinary   = TEXT("Loc.DropA");
	inline constexpr auto LockerAnonymous = TEXT("Loc.Locker");
	inline constexpr auto DropRisky   = TEXT("Loc.RiskyDrop");
	inline constexpr auto PoliceRoute = TEXT("Loc.PoliceRoute");
	inline constexpr auto HostileAlley = TEXT("Loc.HostileAlley");

	/** Contract-facing point ids (drop points bind contracts by id). */
	inline constexpr auto PointDropOrdinary = TEXT("Point.DropA");
	inline constexpr auto PointDropRisky    = TEXT("Point.RiskyDrop");
}
