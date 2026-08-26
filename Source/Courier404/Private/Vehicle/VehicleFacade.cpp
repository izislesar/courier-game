#include "Vehicle/VehicleFacade.h"

UVehicleFacadeComponent::UVehicleFacadeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UVehicleFacadeComponent::CanEnter(AActor* CandidateDriver) const
{
	return IsValid(CandidateDriver) && !Driver.IsValid();
}

bool UVehicleFacadeComponent::Enter(AActor* NewDriver)
{
	if (!CanEnter(NewDriver))
	{
		return false;
	}
	Driver = NewDriver;
	return true;
}

bool UVehicleFacadeComponent::Exit(AActor* Expected)
{
	if (Driver.Get() != Expected || !Driver.IsValid())
	{
		return false;
	}
	Driver = nullptr;
	return true;
}

bool UVehicleFacadeComponent::AttachCargo(AActor* Package)
{
	if (!IsValid(Package) || HasCargo())
	{
		return false;
	}
	CargoPackage = Package;
	return true;
}

bool UVehicleFacadeComponent::DetachCargo(AActor** OutPackage)
{
	if (!HasCargo())
	{
		if (OutPackage)
		{
			*OutPackage = nullptr;
		}
		return false;
	}
	if (OutPackage)
	{
		*OutPackage = CargoPackage.Get();
	}
	CargoPackage = nullptr;
	return true;
}

float UVehicleFacadeComponent::ConsumeFuel(float Liters)
{
	const float Consumed = FMath::Min(FMath::Max(Liters, 0.f), FuelLiters);
	FuelLiters -= Consumed;
	return Consumed;
}

void UVehicleFacadeComponent::NotifyImpact(float RelativeSpeed, AActor* Other)
{
	if (RelativeSpeed > 50.f)
	{
		++ImpactCount;
	}
}
