#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Courier404Contracts.generated.h"

UENUM(BlueprintType)
enum class EContractCategory : uint8
{
	Normal,
	Anonymous
};

UENUM(BlueprintType)
enum class EContractStatus : uint8
{
	Available,
	Accepted,
	PickedUp,
	Completed,
	Failed
};

/** Immutable content definition of one contract. */
USTRUCT(BlueprintType)
struct FContractDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ContractId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) EContractCategory Category = EContractCategory::Normal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName PickupPointId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName DropoffPointId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Reward = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float TimeLimitSeconds = 0.f; // 0 = no limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName CargoId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 RiskLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> Rules;

	/** Returns false and fills OutErrors when the definition is not usable. */
	bool Validate(TArray<FString>& OutErrors) const
	{
		if (ContractId.IsNone())
		{
			OutErrors.Add(TEXT("ContractId is none"));
		}
		if (PickupPointId.IsNone())
		{
			OutErrors.Add(TEXT("PickupPointId is none"));
		}
		if (DropoffPointId.IsNone())
		{
			OutErrors.Add(TEXT("DropoffPointId is none"));
		}
		if (Reward <= 0)
		{
			OutErrors.Add(TEXT("Reward must be positive"));
		}
		if (TimeLimitSeconds < 0.f)
		{
			OutErrors.Add(TEXT("TimeLimitSeconds must be >= 0"));
		}
		return OutErrors.IsEmpty();
	}
};

/** Authoring container so contracts can exist as data assets. */
UCLASS(BlueprintType)
class COURIER404_API UCourier404ContractDefinitionAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contract")
	FContractDefinition Definition;
};

/** Mutable runtime state of one accepted contract instance. */
USTRUCT(BlueprintType)
struct FContractRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName InstanceId;
	UPROPERTY(BlueprintReadOnly) FName ContractId;
	UPROPERTY(BlueprintReadOnly) EContractStatus Status = EContractStatus::Available;
	UPROPERTY(BlueprintReadOnly) float AcceptedAtSimSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly) float PickedUpAtSimSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly) float CompletedAtSimSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly) float FailedAtSimSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly) FName FailureReason;
};
