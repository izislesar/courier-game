#pragma once

#include "CoreMinimal.h"
#include "Contracts/Courier404Contracts.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FCourierOnContractCompleted, const FContractRuntimeState& /*Instance*/, int32 /*Payout*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FCourierOnContractFailed, const FContractRuntimeState& /*Instance*/);

/**
 * Deterministic contract domain service. No UObject/world dependencies;
 * transitions depend only on explicit simulated-time arguments and current state.
 */
class COURIER404_API FCourier404ContractService
{
public:
	/** Registers (or rejects) a definition after validation. Returns false with errors. */
	bool RegisterDefinition(const FContractDefinition& Definition, TArray<FString>& OutErrors);

	const FContractDefinition* FindDefinition(FName ContractId) const;

	/** Creates a runtime instance. Returns none when the contract is unknown. */
	FName Accept(FName ContractId, float SimTimeSeconds);

	bool MarkPickup(FName InstanceId, float SimTimeSeconds);

	/** Requires pickup + matching drop-off; completes exactly once; emits payout event. */
	bool TryDeliver(FName InstanceId, FName DropoffPointId, float SimTimeSeconds, int32& OutPayout);

	bool Fail(FName InstanceId, FName Reason, float SimTimeSeconds);

	bool IsExpired(FName InstanceId, float SimTimeSeconds) const;

	bool ExpireIfPastLimit(FName InstanceId, float SimTimeSeconds);

	const FContractRuntimeState* FindInstance(FName InstanceId) const;

	FCourierOnContractCompleted OnContractCompleted;
	FCourierOnContractFailed OnContractFailed;

private:
	FName MakeInstanceId(FName ContractId, float SimTimeSeconds);

	TMap<FName, FContractDefinition> Definitions;
	TMap<FName, FContractRuntimeState> Instances;
	int32 InstanceCounter = 0;
};
