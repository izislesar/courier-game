#include "Interaction/Courier404DropPoint.h"
#include "Courier404.h"
#include "Contracts/ContractServiceSubsystem.h"
#include "Interaction/CargoCarrierComponent.h"
#include "Interaction/Courier404Package.h"
#include "Components/BoxComponent.h"

ACourier404DropPoint::ACourier404DropPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	SetRootComponent(Zone);
	Zone->InitBoxExtent(FVector(60.f));
	Zone->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	DropPointId = TEXT("Point.DropA");
}

bool ACourier404DropPoint::CanInteract(AActor* Interactor) const
{
	return IsValid(Interactor) && Interactor->FindComponentByClass<UCargoCarrierComponent>() != nullptr;
}

FText ACourier404DropPoint::GetInteractionPrompt(AActor* Interactor) const
{
	return CanInteract(Interactor) ? FText::FromString(TEXT("Deliver here")) : FText::GetEmpty();
}

void ACourier404DropPoint::Interact(AActor* Interactor)
{
	UCargoCarrierComponent* Carrier = Interactor ? Interactor->FindComponentByClass<UCargoCarrierComponent>() : nullptr;
	ACourier404Package* Package = Carrier ? Carrier->GetHeld() : nullptr;
	if (!Package || Package->GetState() != EPackageState::Carried)
	{
		return;
	}

	FCourier404ContractService* Contracts = ResolveContracts();
	if (!Contracts)
	{
		return;
	}

	const FName InstanceId = Contracts->FindPickedUpInstanceForCargo(Package->CargoId);
	if (InstanceId.IsNone())
	{
		UE_LOG(LogCourier404, Log, TEXT("Drop %s: no picked-up contract for cargo %s"),
			*DropPointId.ToString(), *Package->CargoId.ToString());
		return;
	}

	int32 Payout = 0;
	if (!Contracts->TryDeliver(InstanceId, DropPointId, 0.f, Payout))
	{
		UE_LOG(LogCourier404, Log, TEXT("Drop %s: contract rejected delivery"), *DropPointId.ToString());
		return;
	}

	// Exactly-once: consume the delivered package from play state.
	Package->MarkDelivered();
	Carrier->Release();
	UE_LOG(LogCourier404, Log, TEXT("Delivered to %s, payout %d"), *DropPointId.ToString(), Payout);
}

FCourier404ContractService* ACourier404DropPoint::ResolveContracts() const
{
	if (DomainOverride)
	{
		return DomainOverride;
	}
	UWorld* W = GetWorld();
	if (!W || !W->GetGameInstance())
	{
		return nullptr;
	}
	UContractServiceSubsystem* Subsystem = W->GetGameInstance()->GetSubsystem<UContractServiceSubsystem>();
	return Subsystem ? &Subsystem->GetDomain() : nullptr;
}
