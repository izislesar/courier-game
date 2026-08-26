#include "Tools/VerifyDistrictCommandlet.h"
#include "Courier404.h"
#include "Core/Courier404LocationTags.h"
#include "Interaction/Courier404DropPoint.h"
#include "Interaction/Courier404FoodSource.h"
#include "Police/Courier404PoliceTrigger.h"
#include "Encounters/HostileEncounterMarker.h"
#include "Interaction/Courier404Package.h"
#include "Interaction/Courier404Bed.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "GameFramework/PlayerStart.h"


UVerifyDistrictCommandlet::UVerifyDistrictCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UVerifyDistrictCommandlet::Main(const FString& Params)
{
	UWorld* World = LoadObject<UWorld>(nullptr, TEXT("/Game/Maps/District01.District01"));
	if (!World)
	{
		UE_LOG(LogCourier404, Error, TEXT("District01 could not be loaded"));
		return 1;
	}

	const TCHAR* RequiredTags[] = {
		Courier404Locations::Apartment, Courier404Locations::Parking, Courier404Locations::Store,
		Courier404Locations::PickupOrdinary, Courier404Locations::DropOrdinary,
		Courier404Locations::LockerAnonymous, Courier404Locations::DropRisky,
		Courier404Locations::PoliceRoute, Courier404Locations::HostileAlley
	};

	int32 Failures = 0;
	for (const TCHAR* Tag : RequiredTags)
	{
		bool bFound = false;
		for (AActor* Actor : World->PersistentLevel->Actors)
		{
			if (Actor && Actor->ActorHasTag(FName(Tag)))
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			++Failures;
			UE_LOG(LogCourier404, Error, TEXT("Missing landmark tag %s"), Tag);
		}
	}

	bool bHasPlayerStart = false;
	bool bHasSun = false;
	bool bHasSky = false;
	bool bHasFood = false;
	bool bHasBed = false;
	int32 DropPoints = 0;
	for (AActor* Actor : World->PersistentLevel->Actors)
	{
		if (!Actor)
		{
			continue;
		}
		bHasPlayerStart |= Actor->IsA<APlayerStart>();
		bHasSun |= Actor->IsA<ADirectionalLight>();
		bHasSky |= Actor->IsA<ASkyLight>();
		bHasFood |= Actor->IsA<ACourier404FoodSource>();
		bHasBed |= Actor->IsA<ACourier404Bed>();
		DropPoints += Actor->IsA<ACourier404DropPoint>() ? 1 : 0;
	}
	bool bHasPoliceTrigger = false;
	bool bHasHostile = false;
	for (AActor* Actor : World->PersistentLevel->Actors)
	{
		bHasPoliceTrigger |= Actor && Actor->IsA<ACourier404PoliceTrigger>();
		bHasHostile |= Actor && Actor->IsA<ACourier404HostileMarker>();
	}
	if (!bHasPoliceTrigger) { UE_LOG(LogCourier404, Error, TEXT("Missing police trigger")); ++Failures; }
	if (!bHasHostile) { UE_LOG(LogCourier404, Error, TEXT("Missing hostile marker")); ++Failures; }

	int32 Packages = 0;
	for (AActor* Actor : World->PersistentLevel->Actors)
	{
		Packages += Actor && Actor->IsA<ACourier404Package>() ? 1 : 0;
	}
	if (Packages < 1) { UE_LOG(LogCourier404, Error, TEXT("Missing pickup package")); ++Failures; }
	if (!bHasFood) { UE_LOG(LogCourier404, Error, TEXT("Missing food source")); ++Failures; }
	if (!bHasBed) { UE_LOG(LogCourier404, Error, TEXT("Missing bed")); ++Failures; }

	if (!bHasPlayerStart) { UE_LOG(LogCourier404, Error, TEXT("Missing PlayerStart")); ++Failures; }
	if (!bHasSun)        { UE_LOG(LogCourier404, Error, TEXT("Missing sun light")); ++Failures; }
	if (!bHasSky)        { UE_LOG(LogCourier404, Error, TEXT("Missing skylight")); ++Failures; }
	if (DropPoints < 2)  { UE_LOG(LogCourier404, Error, TEXT("Expected >=2 drop points, got %d"), DropPoints); ++Failures; }

	if (Failures == 0)
	{
		UE_LOG(LogCourier404, Log, TEXT("DISTRICT VERIFY PASS: all landmarks, player start, lighting rig and drop points present"));
	}
	return Failures == 0 ? 0 : 1;
}
