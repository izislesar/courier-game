#include "Contracts/ContractServiceSubsystem.h"
#include "Contracts/SliceContracts.h"
#include "Subsystems/SubsystemCollection.h"

void UContractServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Courier404SliceContracts::RegisterDefaults(Service);
}

void UContractServiceSubsystem::Deinitialize()
{
	OnContractCompleted.Clear();
	OnContractFailed.Clear();
	Super::Deinitialize();
}
