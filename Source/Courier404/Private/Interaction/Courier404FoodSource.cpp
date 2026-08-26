#include "Interaction/Courier404FoodSource.h"
#include "Courier404.h"
#include "Core/LifeSubsystem.h"
#include "Components/BoxComponent.h"

ACourier404FoodSource::ACourier404FoodSource()
{
	PrimaryActorTick.bCanEverTick = false;

	Counter = CreateDefaultSubobject<UBoxComponent>(TEXT("Counter"));
	SetRootComponent(Counter);
	Counter->InitBoxExtent(FVector(50.f, 30.f, 40.f));
	Counter->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

bool ACourier404FoodSource::CanInteract(AActor* Interactor) const
{
	return IsValid(Interactor);
}

FText ACourier404FoodSource::GetInteractionPrompt(AActor* Interactor) const
{
	return CanInteract(Interactor)
		? FText::FromString(TEXT("Buy meal"))
		: FText::GetEmpty();
}

void ACourier404FoodSource::Interact(AActor* Interactor)
{
	FCourier404LifeService* Life = ResolveLife();
	if (!Life)
	{
		UE_LOG(LogCourier404, Warning, TEXT("FoodSource: no life service"));
		return;
	}

	// Pre-prod: elapsed time between visits arrives via clock integration.
	if (Life->Eat(/*ElapsedHours*/ 0.f, MealPrice))
	{
		UE_LOG(LogCourier404, Log, TEXT("Bought meal for %d"), MealPrice);
	}
	else
	{
		UE_LOG(LogCourier404, Log, TEXT("Cannot afford meal (%d)"), MealPrice);
	}
}

FCourier404LifeService* ACourier404FoodSource::ResolveLife() const
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
