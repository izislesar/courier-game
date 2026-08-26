#include "Contracts/ContractDomain.h"
#include "Courier404.h"

bool FCourier404ContractService::RegisterDefinition(const FContractDefinition& Definition, TArray<FString>& OutErrors)
{
	if (!Definition.Validate(OutErrors))
	{
		return false;
	}

	if (Definitions.Contains(Definition.ContractId))
	{
		OutErrors.Add(FString::Printf(TEXT("Duplicate ContractId %s"), *Definition.ContractId.ToString()));
		return false;
	}

	Definitions.Add(Definition.ContractId, Definition);
	return true;
}

const FContractDefinition* FCourier404ContractService::FindDefinition(FName ContractId) const
{
	return Definitions.Find(ContractId);
}

FName FCourier404ContractService::Accept(FName ContractId, float SimTimeSeconds)
{
	const FContractDefinition* Definition = FindDefinition(ContractId);
	if (!Definition)
	{
		UE_LOG(LogCourier404, Warning, TEXT("Accept: unknown contract %s"), *ContractId.ToString());
		return NAME_None;
	}

	FContractRuntimeState State;
	State.InstanceId = MakeInstanceId(ContractId, SimTimeSeconds);
	State.ContractId = ContractId;
	State.Status = EContractStatus::Accepted;
	State.AcceptedAtSimSeconds = SimTimeSeconds;
	Instances.Add(State.InstanceId, State);

	return State.InstanceId;
}

bool FCourier404ContractService::MarkPickup(FName InstanceId, float SimTimeSeconds)
{
	FContractRuntimeState* State = Instances.Find(InstanceId);
	if (!State || State->Status != EContractStatus::Accepted)
	{
		return false;
	}

	State->Status = EContractStatus::PickedUp;
	State->PickedUpAtSimSeconds = SimTimeSeconds;
	return true;
}

bool FCourier404ContractService::TryDeliver(FName InstanceId, FName DropoffPointId, float SimTimeSeconds, int32& OutPayout)
{
	OutPayout = 0;

	FContractRuntimeState* State = Instances.Find(InstanceId);
	if (!State || State->Status != EContractStatus::PickedUp)
	{
		return false;
	}

	const FContractDefinition* Definition = FindDefinition(State->ContractId);
	if (!Definition || Definition->DropoffPointId != DropoffPointId)
	{
		return false;
	}

	// Complete exactly once: status flips before broadcast so re-entrant delivery fails.
	State->Status = EContractStatus::Completed;
	State->CompletedAtSimSeconds = SimTimeSeconds;
	OutPayout = Definition->Reward;

	OnContractCompleted.Broadcast(*State, OutPayout);
	return true;
}

bool FCourier404ContractService::Fail(FName InstanceId, FName Reason, float SimTimeSeconds)
{
	FContractRuntimeState* State = Instances.Find(InstanceId);
	if (!State)
	{
		return false;
	}
	if (State->Status == EContractStatus::Completed || State->Status == EContractStatus::Failed)
	{
		return false;
	}

	State->Status = EContractStatus::Failed;
	State->FailureReason = Reason;
	State->FailedAtSimSeconds = SimTimeSeconds;

	OnContractFailed.Broadcast(*State);
	return true;
}

bool FCourier404ContractService::IsExpired(FName InstanceId, float SimTimeSeconds) const
{
	const FContractRuntimeState* State = Instances.Find(InstanceId);
	if (!State || State->Status == EContractStatus::Completed || State->Status == EContractStatus::Failed)
	{
		return false;
	}

	const FContractDefinition* Definition = Definitions.Find(State->ContractId);
	return Definition &&
		Definition->TimeLimitSeconds > 0.f &&
		SimTimeSeconds > State->AcceptedAtSimSeconds + Definition->TimeLimitSeconds;
}

bool FCourier404ContractService::ExpireIfPastLimit(FName InstanceId, float SimTimeSeconds)
{
	if (IsExpired(InstanceId, SimTimeSeconds))
	{
		return Fail(InstanceId, TEXT("Contract.TimeExpired"), SimTimeSeconds);
	}
	return false;
}

const FContractRuntimeState* FCourier404ContractService::FindInstance(FName InstanceId) const
{
	return Instances.Find(InstanceId);
}

void FCourier404ContractService::RestoreInstance(const FContractRuntimeState& State)
{
	if (!Definitions.Contains(State.ContractId))
	{
		UE_LOG(LogCourier404, Warning, TEXT("RestoreInstance: unknown contract %s"), *State.ContractId.ToString());
		return;
	}
	Instances.Add(State.InstanceId, State);
}

FName FCourier404ContractService::FindOpenInstanceForCargo(FName CargoId) const
{
	const FName Active = FindActiveInstanceForCargo(CargoId);
	if (!Active.IsNone())
	{
		return Active;
	}
	return FindPickedUpInstanceForCargo(CargoId);
}

int32 FCourier404ContractService::GetPoliceRiskForCargo(FName CargoId) const
{
	int32 Highest = 0;
	for (const TPair<FName, FContractDefinition>& Pair : Definitions)
	{
		const FContractDefinition& Def = Pair.Value;
		if (Def.CargoId == CargoId && Def.Rules.Contains(TEXT("Rule.PoliceRisk")))
		{
			Highest = FMath::Max(Highest, Def.RiskLevel);
		}
	}
	return Highest;
}

FName FCourier404ContractService::FindActiveInstanceForCargo(FName CargoId) const
{
	const FContractRuntimeState* Best = nullptr;
	for (const TPair<FName, FContractRuntimeState>& Pair : Instances)
	{
		const FContractRuntimeState& State = Pair.Value;
		if (State.Status != EContractStatus::Accepted)
		{
			continue;
		}
		const FContractDefinition* Definition = Definitions.Find(State.ContractId);
		if (!Definition || Definition->CargoId != CargoId)
		{
			continue;
		}
		if (!Best || State.AcceptedAtSimSeconds < Best->AcceptedAtSimSeconds ||
			(State.AcceptedAtSimSeconds == Best->AcceptedAtSimSeconds && State.InstanceId.LexicalLess(Best->InstanceId)))
		{
			Best = &State;
		}
	}
	return Best ? Best->InstanceId : NAME_None;
}

FName FCourier404ContractService::FindPickedUpInstanceForCargo(FName CargoId) const
{
	const FContractRuntimeState* Best = nullptr;
	for (const TPair<FName, FContractRuntimeState>& Pair : Instances)
	{
		const FContractRuntimeState& State = Pair.Value;
		if (State.Status != EContractStatus::PickedUp)
		{
			continue;
		}
		const FContractDefinition* Definition = Definitions.Find(State.ContractId);
		if (!Definition || Definition->CargoId != CargoId)
		{
			continue;
		}
		if (!Best || State.PickedUpAtSimSeconds < Best->PickedUpAtSimSeconds ||
			(State.PickedUpAtSimSeconds == Best->PickedUpAtSimSeconds && State.InstanceId.LexicalLess(Best->InstanceId)))
		{
			Best = &State;
		}
	}
	return Best ? Best->InstanceId : NAME_None;
}

FName FCourier404ContractService::MakeInstanceId(FName ContractId, float SimTimeSeconds)
{
	++InstanceCounter;
	return FName(*FString::Printf(TEXT("%s.%d.%d"),
		*ContractId.ToString(), static_cast<int32>(SimTimeSeconds), InstanceCounter));
}
