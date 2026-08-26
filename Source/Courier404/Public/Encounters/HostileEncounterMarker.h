#pragma once

#include "GameFramework/Actor.h"
#include "Encounters/HostileEncounter.h"
#include "Components/BoxComponent.h"

class UBoxComponent;

#include "HostileEncounterMarker.generated.h"

/**
 * Hostile-courtyard anchor. Runtime AI drives FCourier404HostileEncounter
 * steps from overlap/distance; this actor is the stable world binding point.
 */
UCLASS()
class COURIER404_API ACourier404HostileMarker : public AActor
{
	GENERATED_BODY()

public:
	ACourier404HostileMarker()
	{
		PrimaryActorTick.bCanEverTick = false;

		Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
		SetRootComponent(Zone);
		Zone->InitBoxExtent(FVector(150.f));
		Zone->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Zone;

	/** Attackers configured for this courtyard (group danger). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Courier404")
	int32 AttackerCount = 3;

	FCourier404HostileEncounter& GetEncounter() { return Encounter; }

protected:
	FCourier404HostileEncounter Encounter;
};
