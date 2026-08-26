#include "Core/LifeSubsystem.h"
#include "Time/SimClockSubsystem.h"

void ULifeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Life.BindClock(&Clock);
}

void ULifeSubsystem::Deinitialize()
{
	Life.BindClock(nullptr);
	Super::Deinitialize();
}
