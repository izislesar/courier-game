#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Time/SimClock.h"
#include "SimClockSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FCourierOnNewDay, int32 /*DayIndex*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCourierOnPhaseChanged, ECourier404DayPhase /*Old*/, ECourier404DayPhase /*New*/);

/**
 * Game-facing owner of the simulated clock. Lighting, population and contract
 * availability consume this state; nothing derives progression from wall clock.
 */
UCLASS()
class COURIER404_API USimClockSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	int32 AdvanceHours(float InHours);

	/** Sleep jump to TargetHour (next occurrence). Returns hours jumped. */
	float SleepTo(float TargetHour);

	float GetHourOfDay() const { return Clock.GetHourOfDay(); }
	int32 GetDayIndex() const { return Clock.GetDayIndex(); }
	ECourier404DayPhase GetPhase() const { return Clock.GetPhase(); }
	bool IsNight() const { return Clock.IsNight(); }

	static bool IsHourInWindow(float Hour, float StartHour, float EndHour) { return FCourier404SimClock::IsHourInWindow(Hour, StartHour, EndHour); }

	FCourierOnNewDay OnNewDay;
	FCourierOnPhaseChanged OnPhaseChanged;

	virtual void Deinitialize() override;

private:
	void BroadcastPhase(ECourier404DayPhase Old, ECourier404DayPhase New);

	FCourier404SimClock Clock;
	ECourier404DayPhase LastPhase = ECourier404DayPhase::Morning;
};
