#include "Relationship/Relationship.h"

void FCourier404Relationship::Schedule(int32 DayIndex, float Hour)
{
	PlannedDay = FMath::Max(1, DayIndex);
	PlannedHour = FMath::Fmod(FMath::Max(Hour, 0.f), FCourier404SimClock::HoursPerDay);
	bPlanActive = true;
	bLastPlanMissed = false;
}

bool FCourier404Relationship::IsMeetingNow(const FCourier404SimClock& Clock) const
{
	return bPlanActive &&
		Clock.GetDayIndex() == PlannedDay &&
		Clock.GetHourOfDay() >= PlannedHour - 1.f &&
		Clock.GetHourOfDay() < PlannedHour + GraceHours;
}

bool FCourier404Relationship::Attend(const FCourier404SimClock& Clock)
{
	if (!IsMeetingNow(Clock))
	{
		return false;
	}
	Trust = FMath::Min(1.f, Trust + TrustGainOnAttend);
	LastInteractionDay = Clock.GetDayIndex();
	bPlanActive = false;
	bLastPlanMissed = false;
	return true;
}

bool FCourier404Relationship::EvaluateMissed(const FCourier404SimClock& Clock)
{
	const bool bPastWindow =
		Clock.GetDayIndex() > PlannedDay ||
		(Clock.GetDayIndex() == PlannedDay && Clock.GetHourOfDay() >= PlannedHour + GraceHours);

	if (bPlanActive && bPastWindow)
	{
		Trust = FMath::Max(0.f, Trust - TrustLossOnMiss);
		++MissedCount;
		bPlanActive = false;
		bLastPlanMissed = true;
		return true;
	}
	return false;
}