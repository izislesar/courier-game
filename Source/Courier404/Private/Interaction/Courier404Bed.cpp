#include "Interaction/Courier404Bed.h"
#include "Courier404.h"
#include "Core/LifeSubsystem.h"
#include "Components/BoxComponent.h"

ACourier404Bed::ACourier404Bed()
{
	PrimaryActorTick.bCanEverTick = false;

	Mattress = CreateDefaultSubobject<UBoxComponent>(TEXT("Mattress"));
	SetRootComponent(Mattress);
	Mattress->InitBoxExtent(FVector(45.f, 90.f, 15.f));
	Mattress->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

bool ACourier404Bed::CanInteract(AActor* Interactor) const
{
	return IsValid(Interactor);
}

FText ACourier404Bed::GetInteractionPrompt(AActor* Interactor) const
{
	return CanInteract(Interactor) ? FText::FromString(TEXT("Sleep")) : FText::GetEmpty();
}

void ACourier404Bed::Interact(AActor* Interactor)
{
	FCourier404LifeService* Life = ResolveLife();
	if (!Life)
	{
		UE_LOG(LogCourier404, Warning, TEXT("Bed: no life service"));
		return;
	}
	const float Slept = Life->SleepTo(WakeUpHour);
	UE_LOG(LogCourier404, Log, TEXT("Slept %.1f hours to %.0f:00"), Slept, WakeUpHour);
}

FCourier404LifeService* ACourier404Bed::ResolveLife() const
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
