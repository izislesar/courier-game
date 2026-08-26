#include "Core/Courier404GameMode.h"
#include "Courier404.h"
#include "Player/Courier404Character.h"
#include "UI/Courier404HUD.h"
#include "InputMappingContext.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/LightComponent.h"

ACourier404GameMode::ACourier404GameMode()
{
	DefaultPawnClass = ACourier404Character::StaticClass();
	HUDClass = ACourier404HUD::StaticClass();
	UE_LOG(LogCourier404, Log, TEXT("Courier404GameMode created"));
}

void ACourier404GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Packaged-boot self-audit: greppable SLICE_BOOT lines gate the build script.
	const ACourier404Character* CharCDO = GetDefault<ACourier404Character>();
	const bool bInputOK = IsValid(CharCDO->GetDefaultMappingContext().Get()) &&
		IsValid(CharCDO->GetMoveAction().Get());

	int32 LightComps = 0;
	int32 NonMovable = 0;
	int32 SunCount = 0;
	int32 SkyCount = 0;
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			TArray<ULightComponent*> Lights;
			It->GetComponents<ULightComponent>(Lights);
			for (ULightComponent* L : Lights)
			{
				++LightComps;
				if (L->Mobility != EComponentMobility::Movable)
				{
					++NonMovable;
				}
			}
			SunCount += It->IsA<ADirectionalLight>() ? 1 : 0;
			SkyCount += It->IsA<ASkyLight>() ? 1 : 0;
		}
	}
	const bool bLightsOK = SunCount == 1 && SkyCount == 1 &&
		NonMovable == 0 && LightComps >= 9;

	UE_LOG(LogCourier404, Log, TEXT("SLICE_BOOT INPUT_%s LIGHTS_%s comps=%d nonmovable=%d sun=%d sky=%d"),
		bInputOK ? TEXT("OK") : TEXT("BAD"),
		bLightsOK ? TEXT("OK") : TEXT("BAD"),
		LightComps, NonMovable, SunCount, SkyCount);
}