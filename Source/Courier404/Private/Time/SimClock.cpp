#include "Time/SimClock.h"

void FCourier404SimClock::Restore(int32 InDayIndex, float InHourOfDay)
{
	DayIndex = FMath::Max(1, InDayIndex);
	HourOfDay = FMath::Fmod(FMath::Max(InHourOfDay, 0.f), HoursPerDay);
}

int32 FCourier404SimClock::AdvanceHours(float InHours)
{
	if (InHours <= 0.f)
	{
		return 0;
	}

	const float Total = HourOfDay + InHours;
	const int32 Rollovers = static_cast<int32>(FMath::FloorToInt32(Total / HoursPerDay));

	HourOfDay = FMath::Fmod(Total, HoursPerDay);
	DayIndex += Rollovers;

	return Rollovers;
}

float FCourier404SimClock::SleepTo(float TargetHour)
{
	TargetHour = FMath::Fmod(FMath::Max(TargetHour, 0.f), HoursPerDay);
	if (FMath::IsNearlyEqual(TargetHour, HourOfDay))
	{
		return 0.f;
	}

	float Delta = TargetHour - HourOfDay;
	if (Delta < 0.f)
	{
		Delta += HoursPerDay;
	}

	AdvanceHours(Delta);
	return Delta;
}

ECourier404DayPhase FCourier404SimClock::GetPhase() const
{
	if (HourOfDay >= 6.f && HourOfDay < 11.f)
	{
		return ECourier404DayPhase::Morning;
	}
	if (HourOfDay >= 11.f && HourOfDay < 18.f)
	{
		return ECourier404DayPhase::Day;
	}
	if (HourOfDay >= 18.f && HourOfDay < 22.f)
	{
		return ECourier404DayPhase::Evening;
	}
	return ECourier404DayPhase::Night;
}

bool FCourier404SimClock::IsHourInWindow(float Hour, float StartHour, float EndHour)
{
	Hour = FMath::Fmod(FMath::Max(Hour, 0.f), HoursPerDay);
	StartHour = FMath::Fmod(FMath::Max(StartHour, 0.f), HoursPerDay);
	EndHour = FMath::Fmod(FMath::Max(EndHour, 0.f), HoursPerDay);

	if (StartHour <= EndHour)
	{
		return Hour >= StartHour && Hour < EndHour;
	}
	// Wrapping window (e.g. 20 -> 2).
	return Hour >= StartHour || Hour < EndHour;
}
