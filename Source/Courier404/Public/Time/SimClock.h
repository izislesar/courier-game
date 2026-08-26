#pragma once

#include "CoreMinimal.h"

/** Coarse day phases consumed by lighting, population and availability logic. */
enum class ECourier404DayPhase : uint8
{
	Night,
	Morning,
	Day,
	Evening
};

/**
 * Deterministic simulated clock. All progression flows through explicit
 * Advance/SleepTo calls; wall-clock time is never read.
 *
 * Simulated time is measured in in-game hours (24h day). Day index starts at 1.
 */
class COURIER404_API FCourier404SimClock
{
public:
	static constexpr float SecondsPerHour = 3600.f;
	static constexpr float HoursPerDay = 24.f;

	/** Advances the simulation by InHours (may exceed one day). Returns number of day rollovers. */
	int32 AdvanceHours(float InHours);

	/** Sleep/jump API: moves to TargetHour on the next occurrence. Returns total hours jumped. */
	float SleepTo(float TargetHour);

	float GetHourOfDay() const { return HourOfDay; }
	int32 GetDayIndex() const { return DayIndex; }

	ECourier404DayPhase GetPhase() const;
	bool IsNight() const { return GetPhase() == ECourier404DayPhase::Night; }

	/** Availability window check; supports wrap-around windows (e.g. 20 -> 2). */
	static bool IsHourInWindow(float Hour, float StartHour, float EndHour);

private:
	float HourOfDay = 8.f; // start at 08:00
	int32 DayIndex = 1;
};
