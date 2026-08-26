#include "Core/Courier404GameMode.h"
#include "Courier404.h"
#include "Player/Courier404Character.h"
#include "UI/Courier404HUD.h"
#include "InputMappingContext.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Components/LightComponent.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "TimerManager.h"

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

	// City subsystem health: verify tier-0 roster is populated at runtime.
	CityRoster.Generate();
	const int32 CitizenCount = CityRoster.GetRecords().Num();
	const bool bCityOK = CitizenCount > 0;

	UE_LOG(LogCourier404, Log, TEXT("SLICE_BOOT INPUT_%s LIGHTS_%s CITY_%s=%d comps=%d nonmovable=%d sun=%d sky=%d"),
		bInputOK ? TEXT("OK") : TEXT("BAD"),
		bLightsOK ? TEXT("OK") : TEXT("BAD"),
		bCityOK ? TEXT("OK") : TEXT("FAIL"),
		CitizenCount,
		LightComps, NonMovable, SunCount, SkyCount);

	// Performance capture mode: bounded, non-disruptive frame-time capture.
	if (FParse::Param(FCommandLine::Get(), TEXT("perfcapture")))
	{
		bPerfCaptureEnabled = true;
		StartPerformanceCapture();
	}
}

void ACourier404GameMode::StartPerformanceCapture()
{
	UE_LOG(LogCourier404, Log, TEXT("PERF_CAPTURE: starting bounded capture (%.1f seconds, max %d frames)"), PerfCaptureDurationSeconds, PerfCaptureMaxFrames);
	PerfCaptureStartTime = FPlatformTime::Seconds();
	PerfCaptureLastFrameTime = PerfCaptureStartTime;
	PerfCaptureFrameCount = 0;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PerfCaptureTimerHandle, this, &ACourier404GameMode::StopPerformanceCapture, PerfCaptureDurationSeconds, false);
		World->GetTimerManager().SetTimerForNextTick(this, &ACourier404GameMode::CaptureFrameTiming);
	}
}

void ACourier404GameMode::StopPerformanceCapture()
{
	UE_LOG(LogCourier404, Log, TEXT("PERF_CAPTURE: stopping capture, %d frames captured"), PerfCaptureFrameCount);
	bPerfCaptureEnabled = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PerfCaptureTimerHandle);
	}
	FGenericPlatformMisc::RequestExit(true);
}

void ACourier404GameMode::CaptureFrameTiming()
{
	if (!bPerfCaptureEnabled) return;

	const double CurrentTime = FPlatformTime::Seconds();
	const double DeltaTime = CurrentTime - PerfCaptureLastFrameTime;
	PerfCaptureLastFrameTime = CurrentTime;

	const double FrameMs = DeltaTime * 1000.0;
	const double FPS = (DeltaTime > 0.0) ? (1.0 / DeltaTime) : 0.0;

	UE_LOG(LogCourier404, Log, TEXT("PERF_CAPTURE: frame=%d delta=%.3fms fps=%.1f"), PerfCaptureFrameCount, FrameMs, FPS);

	PerfCaptureFrameCount++;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ACourier404GameMode::CaptureFrameTiming);
	}

	if (PerfCaptureFrameCount >= PerfCaptureMaxFrames)
	{
		StopPerformanceCapture();
	}
}