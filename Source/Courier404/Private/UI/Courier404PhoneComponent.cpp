#include "UI/Courier404PhoneComponent.h"
#include "Contracts/ContractServiceSubsystem.h"
#include "Time/SimClockSubsystem.h"

UCourier404PhoneComponent::UCourier404PhoneComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // bounded update per performance discipline
}

void UCourier404PhoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bOpen)
	{
		Refresh();
	}
}

void UCourier404PhoneComponent::Refresh()
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance())
	{
		return;
	}

	if (!ContractsSubsystem.IsValid())
	{
		ContractsSubsystem = World->GetGameInstance()->GetSubsystem<UContractServiceSubsystem>();
	}
	if (!ClockSubsystem.IsValid())
	{
		ClockSubsystem = World->GetGameInstance()->GetSubsystem<USimClockSubsystem>();
	}

	if (ContractsSubsystem.IsValid())
	{
		ViewModel.Bind(&ContractsSubsystem->GetDomain(),
			ClockSubsystem.IsValid() ? &ClockSubsystem->GetClock() : nullptr);
	}
	ViewModel.Refresh();
}
