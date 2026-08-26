#pragma once

#include "CoreMinimal.h"
#include "Time/SimClock.h"

/**
 * Small persistent relationship state for one contact. Consequences emerge
 * from the schedule (time passing while elsewhere), never from popups.
 */
class COURIER404_API FCourier404Relationship
{
public:
	static constexpr float TrustGainOnAttend = 0.15f;
	static constexpr float TrustLossOnMiss = 0.25f;
	static constexpr float GraceHours = 2.f;

	/** Plans an evening meeting on DayIndex at Hour (e.g. day 1, 20:00). */
	void Schedule(int32 DayIndex, float Hour);

	/** True when the player may attend right now (planned day, within window). */
	bool IsMeetingNow(const FCourier404SimClock& Clock) const;

	/** Attending inside the window: trust rises, plan clears. Returns true when counted. */
	bool Attend(const FCourier404SimClock& Clock);

	/**
	 * Lazy evaluation: once the window has passed unattended, records the miss
	 * (trust loss + counter). Idempotent until a new plan is scheduled.
	 */
	bool EvaluateMissed(const FCourier404SimClock& Clock);

	float GetTrust() const { return Trust; }
	int32 GetMissedCount() const { return MissedCount; }
	bool HasPendingPlan() const { return bPlanActive; }
	bool WasLastPlanMissed() const { return bLastPlanMissed; }
	FString GetContactName() const { return TEXT("Lena"); }

	/** Persistence restore. */
	void Restore(float InTrust, int32 InMissedCount, bool bInLastPlanMissed,
		bool bInPlanActive, int32 InPlannedDay, float InPlannedHour)
	{
		Trust = FMath::Clamp(InTrust, 0.f, 1.f);
		MissedCount = FMath::Max(0, InMissedCount);
		bLastPlanMissed = bInLastPlanMissed;
		bPlanActive = bInPlanActive;
		PlannedDay = InPlannedDay;
		PlannedHour = InPlannedHour;
	}

	int32 GetLastInteractionDay() const { return LastInteractionDay; }

private:
	float Trust = 0.7f;
	int32 MissedCount = 0;
	int32 LastInteractionDay = 0;
	bool bLastPlanMissed = false;
	bool bPlanActive = false;
	int32 PlannedDay = 1;
	float PlannedHour = 20.f;
};