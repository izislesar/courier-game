#include "Time/SimClockSubsystem.h"

int32 USimClockSubsystem::AdvanceHours(float InHours)
{
	const ECourier404DayPhase Old = Clock.GetPhase();
	const int32 Rollovers = Clock.AdvanceHours(InHours);

	if (Rollovers > 0)
	{
		OnNewDay.Broadcast(Clock.GetDayIndex());
	}
	BroadcastPhase(Old, Clock.GetPhase());

	return Rollovers;
}

float USimClockSubsystem::SleepTo(float TargetHour)
{
	const ECourier404DayPhase Old = Clock.GetPhase();
	const float Jumped = Clock.SleepTo(TargetHour);
	BroadcastPhase(Old, Clock.GetPhase());
	return Jumped;
}

void USimClockSubsystem::BroadcastPhase(ECourier404DayPhase Old, ECourier404DayPhase New)
{
	if (Old != New)
	{
		OnPhaseChanged.Broadcast(Old, New);
	}
	LastPhase = New;
}

void USimClockSubsystem::Deinitialize()
{
	OnNewDay.Clear();
	OnPhaseChanged.Clear();
	Super::Deinitialize();
}
