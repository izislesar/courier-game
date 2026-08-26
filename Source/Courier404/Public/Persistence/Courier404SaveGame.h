#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Contracts/Courier404Contracts.h"
#include "Courier404SaveGame.generated.h"

/** One serialized runtime contract instance. */
USTRUCT()
 struct FCourier404ContractRecord
{
	GENERATED_BODY()

	UPROPERTY() FName InstanceId;
	UPROPERTY() FName ContractId;
	UPROPERTY() EContractStatus Status = EContractStatus::Available;
	UPROPERTY() float AcceptedAtSimSeconds = 0.f;
	UPROPERTY() float PickedUpAtSimSeconds = 0.f;
	UPROPERTY() float CompletedAtSimSeconds = 0.f;
	UPROPERTY() float FailedAtSimSeconds = 0.f;
	UPROPERTY() FName FailureReason;
};

/** Versioned save payload for the pre-production slice. */
UCLASS()
class COURIER404_API UCourier404SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentVersion = 1;

	UPROPERTY() int32 SaveVersion = CurrentVersion;

	// Money
	UPROPERTY() int32 Money = 0;

	// Simulated clock
	UPROPERTY() int32 DayIndex = 1;
	UPROPERTY() float HourOfDay = 8.f;

	// Needs
	UPROPERTY() float Hunger01 = 1.f;
	UPROPERTY() float Fatigue01 = 1.f;
	UPROPERTY() float Health01 = 1.f;
	UPROPERTY() float TimeStarvingHours = 0.f;

	// Relationship
	UPROPERTY() float RelationshipTrust = 0.7f;
	UPROPERTY() int32 RelationshipMissedCount = 0;
	UPROPERTY() int32 RelationshipLastInteractionDay = 0;
	UPROPERTY() bool bRelationshipLastPlanMissed = false;
	UPROPERTY() bool bRelationshipPlanActive = false;
	UPROPERTY() int32 RelationshipPlannedDay = 1;
	UPROPERTY() float RelationshipPlannedHour = 20.f;

	// Consequence flags (police/hostile history)
	UPROPERTY() TMap<FName, float> ConsequenceFlagsUntil;

	// Contracts
	UPROPERTY() TArray<FCourier404ContractRecord> ContractRecords;

	// Player resume
	UPROPERTY() FVector PlayerLocation = FVector::ZeroVector;
	UPROPERTY() FRotator PlayerRotation = FRotator::ZeroRotator;
};
