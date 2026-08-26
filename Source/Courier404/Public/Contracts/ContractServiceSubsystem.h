#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Contracts/ContractDomain.h"
#include "ContractServiceSubsystem.generated.h"

/**
 * World-facing owner of the contract domain service. Presentation and world
 * actors bind here; all semantics live in FCourier404ContractService.
 */
UCLASS()
class COURIER404_API UContractServiceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool RegisterDefinition(const FContractDefinition& Definition, TArray<FString>& OutErrors)
	{
		return Service.RegisterDefinition(Definition, OutErrors);
	}

	const FContractDefinition* FindDefinition(FName ContractId) const { return Service.FindDefinition(ContractId); }

	FName Accept(FName ContractId, float SimTimeSeconds) { return Service.Accept(ContractId, SimTimeSeconds); }

	bool MarkPickup(FName InstanceId, float SimTimeSeconds) { return Service.MarkPickup(InstanceId, SimTimeSeconds); }

	bool TryDeliver(FName InstanceId, FName DropoffPointId, float SimTimeSeconds, int32& OutPayout)
	{
		return Service.TryDeliver(InstanceId, DropoffPointId, SimTimeSeconds, OutPayout);
	}

	bool Fail(FName InstanceId, FName Reason, float SimTimeSeconds) { return Service.Fail(InstanceId, Reason, SimTimeSeconds); }

	bool IsExpired(FName InstanceId, float SimTimeSeconds) const { return Service.IsExpired(InstanceId, SimTimeSeconds); }

	bool ExpireIfPastLimit(FName InstanceId, float SimTimeSeconds) { return Service.ExpireIfPastLimit(InstanceId, SimTimeSeconds); }

	const FContractRuntimeState* FindInstance(FName InstanceId) const { return Service.FindInstance(InstanceId); }

	FCourierOnContractCompleted OnContractCompleted;
	FCourierOnContractFailed OnContractFailed;

	virtual void Deinitialize() override;

private:
	FCourier404ContractService Service;
};
