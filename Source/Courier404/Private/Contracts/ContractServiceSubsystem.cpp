#include "Contracts/ContractServiceSubsystem.h"

void UContractServiceSubsystem::Deinitialize()
{
	OnContractCompleted.Clear();
	OnContractFailed.Clear();
	Super::Deinitialize();
}
