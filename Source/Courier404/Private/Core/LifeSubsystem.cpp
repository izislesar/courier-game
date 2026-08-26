#include "Core/LifeSubsystem.h"
#include "Time/SimClockSubsystem.h"
#include "Contracts/ContractServiceSubsystem.h"

void ULifeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Life.BindClock(&Clock);

	// Economy bridge: contract payouts credit the player wallet exactly once
	// (the service guarantees single completion events).
	if (UContractServiceSubsystem* Contracts = Collection.InitializeDependency<UContractServiceSubsystem>())
	{
		Contracts->OnContractCompleted.AddLambda([this](const FContractRuntimeState&, int32 Payout)
		{
			Life.GetWallet().Add(Payout);
		});
	}
}

void ULifeSubsystem::Deinitialize()
{
	Life.BindClock(nullptr);
	Super::Deinitialize();
}
