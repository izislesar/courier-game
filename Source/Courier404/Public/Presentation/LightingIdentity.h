#pragma once

#include "CoreMinimal.h"
#include "Time/SimClock.h"

/**
 * Cohesive day/night lighting identity (no dynamic GI: baseline renderer per
 * performance-budget.md). Maps the simulated phase to concrete light values;
 * presentation components consume this table.
 */
struct COURIER404_API FCourier404PhaseLook
{
	float SunIntensity = 2.5f;
	FLinearColor SunColor = FLinearColor::White;
	float SkyIntensity = 1.f;
	float LampIntensityScale = 0.f; // decorative street lights off by day
	float ExposureMin = 0.7f;
	float ExposureMax = 1.2f;
};

class COURIER404_API FCourier404LightingIdentity
{
public:
	static FCourier404PhaseLook GetLook(ECourier404DayPhase Phase);

	/** Interpolated look for an arbitrary hour (smooth transitions). */
	static FCourier404PhaseLook GetLookForHour(float HourOfDay);
};
