#include "Interaction/Courier404Girlfriend.h"
#include "Courier404.h"
#include "Core/LifeSubsystem.h"
#include "Components/BoxComponent.h"

ACourier404Girlfriend::ACourier404Girlfriend()
{
	PrimaryActorTick.bCanEverTick = false;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	SetRootComponent(Zone);
	Zone->InitBoxExtent(FVector(60.f));
	Zone->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

bool ACourier404Girlfriend::CanInteract(AActor* Interactor) const
{
	const FCourier404LifeService* Life = ResolveLife();
	return IsValid(Interactor) && Life && Life->GetRelationship().IsMeetingNow(Life->GetClock() ? *Life->GetClock() : FCourier404SimClock());
}

FText ACourier404Girlfriend::GetInteractionPrompt(AActor* Interactor) const
{
	return CanInteract(Interactor) ? FText::FromString(TEXT("Meet Lena")) : FText::GetEmpty();
}

void ACourier404Girlfriend::Interact(AActor* Interactor)
{
	FCourier404LifeService* Life = ResolveLife();
	if (!Life || !Life->GetClock())
	{
		return;
	}
	if (Life->GetRelationship().Attend(*Life->GetClock()))
	{
		UE_LOG(LogCourier404, Log, TEXT("Met %s - trust now %.2f"),
			*Life->GetRelationship().GetContactName(), Life->GetRelationship().GetTrust());
	}
}

FCourier404LifeService* ACourier404Girlfriend::ResolveLife() const
{
	if (LifeOverride)
	{
		return LifeOverride;
	}
	UWorld* W = GetWorld();
	if (!W || !W->GetGameInstance())
	{
		return nullptr;
	}
	ULifeSubsystem* Subsystem = W->GetGameInstance()->GetSubsystem<ULifeSubsystem>();
	return Subsystem ? &Subsystem->GetLife() : nullptr;
}
