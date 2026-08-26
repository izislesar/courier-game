#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Player/Courier404Character.h"
#include "Vehicle/Courier404VehiclePawn.h"
#include "InputMappingContext.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/LightComponent.h"
#include "Presentation/LightingIdentity.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404CookedInputRefs,
	"Courier404.Packaged.InputHardReferencesResolve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404CookedInputRefs::RunTest(const FString& Parameters)
{
	// Regression for packaged-build dead input: the mapping contexts MUST be
	// hard-referenced from CDOs (soft paths are not followed by the cooker).
	const ACourier404Character* CharCDO = GetDefault<ACourier404Character>();
	TestTrue(TEXT("character IMC hard ref"), IsValid(CharCDO->GetDefaultMappingContext().Get()));
	TestTrue(TEXT("move action"), IsValid(CharCDO->GetMoveAction().Get()));
	TestTrue(TEXT("look action"), IsValid(CharCDO->GetLookAction().Get()));
	TestTrue(TEXT("interact action"), IsValid(CharCDO->GetInteractAction().Get()));
	TestTrue(TEXT("phone toggle"), IsValid(CharCDO->GetPhoneToggleAction().Get()));

	const ACourier404VehiclePawn* VehCDO = GetDefault<ACourier404VehiclePawn>();
	TestTrue(TEXT("vehicle IMC hard ref"), IsValid(VehCDO->GetDriveMappingContext().Get()));
	TestTrue(TEXT("drive action"), IsValid(VehCDO->GetDriveAction().Get()));
	TestTrue(TEXT("brake action"), IsValid(VehCDO->GetBrakeAction().Get()));

	// Direct path load: catches silent content loss even without CDO wiring.
	for (const TCHAR* Path : {
		TEXT("/Game/Input/IMC_CourierDefault.IMC_CourierDefault"),
		TEXT("/Game/Input/IMC_CourierDrive.IMC_CourierDrive") })
	{
		UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, Path);
		if (!TestNotNull(FString::Printf(TEXT("asset %s loads"), Path), Asset))
		{
			return false;
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404DistrictLightAudit,
	"Courier404.Packaged.DistrictLightingAudit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404DistrictLightAudit::RunTest(const FString& Parameters)
{
	UWorld* World = LoadObject<UWorld>(nullptr, TEXT("/Game/Maps/District01.District01"));
	if (!TestNotNull(TEXT("district map loads"), World))
	{
		return false;
	}

	int32 Sun = 0, Sky = 0, Lamps = 0, NonMovable = 0;
	float SunIntensity = -1.f, SkyIntensity = -1.f;

	for (AActor* Actor : World->PersistentLevel->Actors)
	{
		if (!Actor) { continue; }

		if (ADirectionalLight* D = Cast<ADirectionalLight>(Actor))
		{
			++Sun;
			SunIntensity = D->GetLightComponent()->Intensity;
			if (D->GetLightComponent()->Mobility != EComponentMobility::Movable) { ++NonMovable; }
		}
		else if (ASkyLight* S = Cast<ASkyLight>(Actor))
		{
			++Sky;
			SkyIntensity = S->GetLightComponent()->Intensity;
			if (S->GetLightComponent()->Mobility != EComponentMobility::Movable) { ++NonMovable; }
		}
		TArray<ULightComponent*> Lights;
		Actor->GetComponents<ULightComponent>(Lights);
		for (ULightComponent* L : Lights)
		{
			++Lamps;
			if (L->Mobility != EComponentMobility::Movable) { ++NonMovable; }
		}
	}

	TestEqual(TEXT("exactly one sun"), Sun, 1);
	TestEqual(TEXT("exactly one skylight"), Sky, 1);
	TestTrue(FString::Printf(TEXT("street/apartment lamps present (%d)"), Lamps), Lamps >= 9);
	TestEqual(TEXT("no stationary/static lights (bake dependency)"), NonMovable, 0);

	// Authored rig stores the daytime baseline; runtime drives it via identity.
	const FCourier404PhaseLook Day = FCourier404LightingIdentity::GetLook(ECourier404DayPhase::Day);
	TestEqual(TEXT("sun matches day identity"), SunIntensity, Day.SunIntensity);
	TestTrue(TEXT("sky contributes ambient"), SkyIntensity > 0.f);
	TestTrue(TEXT("daylight believable (>=6)"), SunIntensity >= 6.f);

	return true;
}

#endif
