#include "Presentation/LightingIdentity.h"

namespace
{
	FCourier404PhaseLook MakeLook(float Sun, FLinearColor Color, float Sky, float Lamps,
		float ExpMin, float ExpMax)
	{
		FCourier404PhaseLook Look;
		Look.SunIntensity = Sun;
		Look.SunColor = MoveTemp(Color);
		Look.SkyIntensity = Sky;
		Look.LampIntensityScale = Lamps;
		Look.ExposureMin = ExpMin;
		Look.ExposureMax = ExpMax;
		return Look;
	}
}

FCourier404PhaseLook FCourier404LightingIdentity::GetLook(ECourier404DayPhase Phase)
{
	switch (Phase)
	{
	case ECourier404DayPhase::Morning:
		return MakeLook(6.5f, FLinearColor(1.f, 0.85f, 0.7f), 1.6f, 0.25f, 0.55f, 1.05f);
	case ECourier404DayPhase::Day:
		return MakeLook(10.f, FLinearColor(1.f, 0.95f, 0.88f), 2.f, 0.f, 0.7f, 1.2f);
	case ECourier404DayPhase::Evening:
		return MakeLook(3.f, FLinearColor(1.f, 0.6f, 0.35f), 0.9f, 0.8f, 0.45f, 0.95f);
	case ECourier404DayPhase::Night:
	default:
		// Dark but readable: moonlight key + strong practicals.
		return MakeLook(0.35f, FLinearColor(0.5f, 0.62f, 1.f), 0.35f, 1.f, 0.28f, 0.65f);
	}
}

namespace
{
	const TArray<FCourier404PhaseLook>& Keyframes()
	{
		static const TArray<FCourier404PhaseLook> Keys = {
			FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Night),    // 00
			FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Night),    // 06
			FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Morning),  // 06-11
			FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Day),      // 11-18
			FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Evening),  // 18-22
			FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Night),    // 22-24
		};
		return Keys;
	}

	float Lerp(float A, float B, float T) { return A + (B - A) * T; }
	FLinearColor LerpColor(const FLinearColor& A, const FLinearColor& B, float T)
	{
		return FLinearColor(
			Lerp(A.R, B.R, T), Lerp(A.G, B.G, T), Lerp(A.B, B.B, T), Lerp(A.A, B.A, T));
	}
}

FCourier404PhaseLook FCourier404LightingIdentity::GetLookForHour(float HourOfDay)
{
	HourOfDay = FMath::Fmod(FMath::Max(HourOfDay, 0.f), 24.f);

	// Segment boundaries: [0,6) night, [6,11) morning, [11,18) day, [18,22) evening, [22,24] night.
	float T = 0.f;
	int32 From = 0, To = 0;

	if (HourOfDay < 6.f)
	{
		From = 0; To = 1; T = HourOfDay / 6.f;
	}
	else if (HourOfDay < 11.f)
	{
		From = 1; To = 2; T = (HourOfDay - 6.f) / 5.f;
	}
	else if (HourOfDay < 18.f)
	{
		From = 2; To = 3; T = (HourOfDay - 11.f) / 7.f;
	}
	else if (HourOfDay < 22.f)
	{
		From = 3; To = 4; T = (HourOfDay - 18.f) / 4.f;
	}
	else
	{
		From = 4; To = 5; T = (HourOfDay - 22.f) / 2.f;
	}

	const FCourier404PhaseLook& A = Keyframes()[From];
	const FCourier404PhaseLook& B = Keyframes()[To];

	FCourier404PhaseLook Out;
	Out.SunIntensity = Lerp(A.SunIntensity, B.SunIntensity, T);
	Out.SunColor = LerpColor(A.SunColor, B.SunColor, T);
	Out.SkyIntensity = Lerp(A.SkyIntensity, B.SkyIntensity, T);
	Out.LampIntensityScale = Lerp(A.LampIntensityScale, B.LampIntensityScale, T);
	Out.ExposureMin = Lerp(A.ExposureMin, B.ExposureMin, T);
	Out.ExposureMax = Lerp(A.ExposureMax, B.ExposureMax, T);
	return Out;
}