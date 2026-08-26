#pragma once

#include "Components/ActorComponent.h"
#include "UI/PhoneViewModel.h"
#include "Courier404PhoneComponent.generated.h"

class UContractServiceSubsystem;
class USimClockSubsystem;

/**
 * Player-carried phone state. Thin bridge: resolves domain services, exposes
 * the view model to the HUD and input.
 */
UCLASS(ClassGroup = (Courier404), meta = (BlueprintSpawnableComponent))
class COURIER404_API UCourier404PhoneComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCourier404PhoneComponent();

	void ToggleOpen() { bOpen = !bOpen; }
	bool IsOpen() const { return bOpen; }

	FCourier404PhoneViewModel& GetViewModel() { return ViewModel; }
	const FCourier404PhoneViewModel& GetViewModel() const { return ViewModel; }

	/** Refresh offers from current domain state; call when opening or on demand. */
	void Refresh();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bOpen = false;
	FCourier404PhoneViewModel ViewModel;

	UPROPERTY()
	TWeakObjectPtr<UContractServiceSubsystem> ContractsSubsystem;

	UPROPERTY()
	TWeakObjectPtr<USimClockSubsystem> ClockSubsystem;
};
