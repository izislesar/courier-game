#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "City/CitySimulation.h"
#include "Courier404GameMode.generated.h"

/**
 * Minimal game mode proving the build/codegen pipeline.
 * Gameplay rules live in domain systems/subsystems, not here.
 */
UCLASS(Config = Game)
class COURIER404_API ACourier404GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACourier404GameMode();
	virtual void BeginPlay() override;

private:
	FCourier404CityRoster CityRoster;

	/** Performance capture state */
	bool bPerfCaptureEnabled = false;
	FTimerHandle PerfCaptureTimerHandle;
	double PerfCaptureStartTime = 0.0;
	double PerfCaptureLastFrameTime = 0.0;
	int32 PerfCaptureFrameCount = 0;
	static constexpr double PerfCaptureDurationSeconds = 10.0;
	static constexpr int32 PerfCaptureMaxFrames = 600;

	void StartPerformanceCapture();
	void StopPerformanceCapture();
	void CaptureFrameTiming();
};
