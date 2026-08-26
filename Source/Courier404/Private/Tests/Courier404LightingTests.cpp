#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Presentation/LightingIdentity.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404LightingIdentityTest,
	"Courier404.Presentation.DayNightLightingIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404LightingIdentityTest::RunTest(const FString& Parameters)
{
	// Night is dark but practicals run at full: night stays readable.
	const FCourier404PhaseLook Night = FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Night);
	TestEqual(TEXT("night sun is moonlight-dim"), Night.SunIntensity, 0.08f);
	TestEqual(TEXT("night lamps at full"), Night.LampIntensityScale, 1.f);
	TestTrue(TEXT("night exposure tighter"), Night.ExposureMin < Night.ExposureMax);

	// Day: sun dominates, decorative lamps off.
	const FCourier404PhaseLook Day = FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Day);
	TestEqual(TEXT("day sun bright"), Day.SunIntensity, 2.5f);
	TestEqual(TEXT("day lamps off"), Day.LampIntensityScale, 0.f);

	// Interpolation: mid-morning sits strictly between night and day values.
	const FCourier404PhaseLook Nine = FCourier404LightingIdentity::GetLookForHour(9.f);
	TestTrue(TEXT("9am sun between night and day"),
		Nine.SunIntensity > 0.08f && Nine.SunIntensity < 2.5f);
	TestTrue(TEXT("9am lamps fading out"),
		Nine.LampIntensityScale > 0.f && Nine.LampIntensityScale < 1.f);

	// Continuity across midnight boundary.
	const FCourier404PhaseLook Late23 = FCourier404LightingIdentity::GetLookForHour(23.99f);
	const FCourier404PhaseLook Early00 = FCourier404LightingIdentity::GetLookForHour(0.01f);
	TestTrue(TEXT("midnight continuity"),
		FMath::Abs(Late23.SunIntensity - Early00.SunIntensity) < 0.05f &&
		FMath::Abs(Late23.LampIntensityScale - Early00.LampIntensityScale) < 0.05f);

	return true;
}

#endif
