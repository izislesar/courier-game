#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Persistence/Courier404SaveGame.h"
#include "PersistenceSubsystem.generated.h"

class UContractServiceSubsystem;
class ULifeSubsystem;

/**
 * Owns save/load of durable slice state. Capture/apply are pure domain
 * operations (unit-testable); slot IO wraps USaveGame via UGameplayStatics.
 */
UCLASS()
class COURIER404_API UPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr const TCHAR* DefaultSlot = TEXT("Courier404");

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Fills SaveObject from live state. Returns false on missing dependencies. */
	bool CaptureTo(UCourier404SaveGame* SaveObject) const;

	/**
	 * Applies a payload to live state. Refuses incompatible versions and never
	 * duplicates rewards: contract records restore exactly as saved.
	 */
	bool ApplyFrom(const UCourier404SaveGame* SaveObject);

	bool SaveToSlot(const TCHAR* SlotName = DefaultSlot);
	bool LoadFromSlot(const TCHAR* SlotName = DefaultSlot);

private:
	UPROPERTY()
	TWeakObjectPtr<UContractServiceSubsystem> Contracts;

	UPROPERTY()
	TWeakObjectPtr<ULifeSubsystem> Life;
};
