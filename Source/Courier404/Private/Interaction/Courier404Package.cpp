#include "Interaction/Courier404Package.h"
#include "Contracts/ContractDomain.h"
#include "Contracts/ContractServiceSubsystem.h"
#include "Interaction/CargoCarrierComponent.h"
#include "Vehicle/VehicleFacade.h"
#include "Components/BoxComponent.h"

ACourier404Package::ACourier404Package()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SetRootComponent(BoxCollision);
	BoxCollision->InitBoxExtent(FVector(20.f));
	BoxCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void ACourier404Package::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

bool ACourier404Package::CanInteract(AActor* Interactor) const
{
	return State != EPackageState::Delivered && IsValid(Interactor);
}

FText ACourier404Package::GetInteractionPrompt(AActor* Interactor) const
{
	if (!CanInteract(Interactor))
	{
		return FText::GetEmpty();
	}
	return State == EPackageState::Carried ? FText::FromString(TEXT("Put down")) : FText::FromString(TEXT("Take package"));
}

void ACourier404Package::Interact(AActor* Interactor)
{
	switch (State)
	{
	case EPackageState::Free:
		TryPickup(Interactor);
		break;
	case EPackageState::Carried:
		Drop();
		break;
	default:
		break;
	}
}

bool ACourier404Package::TryPickup(AActor* PlayerInteractor)
{
	if (State != EPackageState::Free || !IsValid(PlayerInteractor))
	{
		return false;
	}
	FCourier404ContractService* Contracts = ResolveContracts();
	if (!Contracts)
	{
		return false;
	}

	// Bind to an Accepted instance (first physical pickup) or to the already
	// picked-up instance (re-pickup after putting the parcel down).
	FName InstanceId = Contracts->FindActiveInstanceForCargo(CargoId);
	const float Now = 0.f; // simulated time arrives with the clock integration issue
	if (!InstanceId.IsNone())
	{
		if (!Contracts->MarkPickup(InstanceId, Now))
		{
			return false; // exactly-once guard
		}
	}
	else
	{
		InstanceId = Contracts->FindPickedUpInstanceForCargo(CargoId);
		if (InstanceId.IsNone())
		{
			return false; // no active job for this cargo
		}
	}

	UCargoCarrierComponent* Carrier = PlayerInteractor->FindComponentByClass<UCargoCarrierComponent>();
	if (!Carrier || !Carrier->Hold(this))
	{
		return false; // carrier full: contract pickup must not double-count
	}

	State = EPackageState::Carried;
	Holder = PlayerInteractor;
	SetActorEnableCollision(false);
	AttachToActor(PlayerInteractor, FAttachmentTransformRules::KeepRelativeTransform);
	SetActorRelativeLocation(FVector(35.f, 0.f, -10.f));
	return true;
}

void ACourier404Package::MarkDelivered()
{
	State = EPackageState::Delivered;
	Holder = nullptr;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

bool ACourier404Package::Drop()
{
	if (State != EPackageState::Carried)
	{
		return false;
	}

	AActor* Carrier = Holder.Get();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if (Carrier)
	{
		if (UCargoCarrierComponent* Component = Carrier->FindComponentByClass<UCargoCarrierComponent>())
		{
			// Only clear if still holding this package.
			if (Component->GetHeld() == this)
			{
				Component->Release();
			}
		}
	}
	SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 30.f), true);
	SetActorEnableCollision(true);
	State = EPackageState::Free;
	Holder = nullptr;
	return true;
}

bool ACourier404Package::PlaceInVehicle(AActor* Vehicle)
{
	if (State != EPackageState::Carried || !IsValid(Vehicle))
	{
		return false;
	}
	UVehicleFacadeComponent* Facade = Vehicle->FindComponentByClass<UVehicleFacadeComponent>();
	if (!Facade || !Facade->AttachCargo(this))
	{
		return false;
	}

	AActor* Carrier = Holder.Get();
	if (Carrier)
	{
		if (UCargoCarrierComponent* Component = Carrier->FindComponentByClass<UCargoCarrierComponent>())
		{
			if (Component->GetHeld() == this)
			{
				Component->Release();
			}
		}
	}
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	AttachToComponent(Vehicle->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(FVector(-90.f, 0.f, 20.f)); // trunk socket position
	SetActorEnableCollision(false);
	State = EPackageState::InVehicle;
	Holder = Vehicle;
	return true;
}

bool ACourier404Package::RetrieveFromVehicle(AActor* PlayerInteractor)
{
	if (State != EPackageState::InVehicle || !IsValid(PlayerInteractor))
	{
		return false;
	}
	AActor* Vehicle = Holder.Get();
	UVehicleFacadeComponent* Facade = Vehicle ? Vehicle->FindComponentByClass<UVehicleFacadeComponent>() : nullptr;
	if (!Facade)
	{
		return false;
	}
	AActor* Out = nullptr;
	if (!Facade->DetachCargo(&Out) || Out != this)
	{
		return false;
	}
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	return TryPickup(PlayerInteractor); // reuses carrier + contract exactly-once path
}

FCourier404ContractService* ACourier404Package::ResolveContracts() const
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