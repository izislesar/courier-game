#include "Interaction/CargoCarrierComponent.h"
#include "Interaction/Courier404Package.h"

bool UCargoCarrierComponent::Hold(ACourier404Package* Package)
{
	if (!IsValid(Package) || Held.IsValid())
	{
		return false;
	}
	Held = Package;
	return true;
}

ACourier404Package* UCargoCarrierComponent::Release()
{
	ACourier404Package* Out = Held.Get();
	Held = nullptr;
	return Out;
}
