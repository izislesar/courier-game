#include "Police/Courier404PoliceTrigger.h"
#include "Components/BoxComponent.h"

ACourier404PoliceTrigger::ACourier404PoliceTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	SetRootComponent(Zone);
	Zone->InitBoxExtent(FVector(200.f));
	Zone->SetCollisionProfileName(TEXT("Trigger"));
	Zone->SetGenerateOverlapEvents(true);
}

void ACourier404PoliceTrigger::BeginPlay()
{
	Super::BeginPlay();
	Zone->OnComponentBeginOverlap.AddDynamic(this, &ACourier404PoliceTrigger::HandleEnter);
}

void ACourier404PoliceTrigger::HandleEnter(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	OnStopRequested.Broadcast(static_cast<ECourier404StopReason>(Reason));
}