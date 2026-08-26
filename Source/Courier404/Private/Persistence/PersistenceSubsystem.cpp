#include "Persistence/PersistenceSubsystem.h"
#include "Courier404.h"
#include "Contracts/ContractServiceSubsystem.h"
#include "Core/LifeSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UPersistenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Contracts = Collection.InitializeDependency<UContractServiceSubsystem>();
	Life = Collection.InitializeDependency<ULifeSubsystem>();
}

bool UPersistenceSubsystem::CaptureTo(UCourier404SaveGame* SaveObject) const
{
	const UContractServiceSubsystem* ContractsSub = Contracts.Get();
	const ULifeSubsystem* LifeSub = Life.Get();
	if (!SaveObject || !ContractsSub || !LifeSub)
	{
		return false;
	}

	const FCourier404LifeService& SrcLife = LifeSub->GetLife();
	const FCourier404SimClock* SrcClock = SrcLife.GetClock();

	SaveObject->SaveVersion = UCourier404SaveGame::CurrentVersion;
	SaveObject->Money = SrcLife.GetWallet().GetBalance();
	SaveObject->DayIndex = SrcClock ? SrcClock->GetDayIndex() : 1;
	SaveObject->HourOfDay = SrcClock ? SrcClock->GetHourOfDay() : 8.f;
	SaveObject->Hunger01 = SrcLife.GetNeeds().GetHunger01();
	SaveObject->Fatigue01 = SrcLife.GetNeeds().GetFatigue01();
	SaveObject->Health01 = SrcLife.GetNeeds().GetHealth01();
	SaveObject->TimeStarvingHours = SrcLife.GetNeeds().GetTimeStarving();

	SaveObject->ContractRecords.Reset();
	for (const TPair<FName, FContractRuntimeState>& Pair : ContractsSub->GetDomain().GetInstances())
	{
		const FContractRuntimeState& S = Pair.Value;
		FCourier404ContractRecord R;
		R.InstanceId = S.InstanceId;
		R.ContractId = S.ContractId;
		R.Status = S.Status;
		R.AcceptedAtSimSeconds = S.AcceptedAtSimSeconds;
		R.PickedUpAtSimSeconds = S.PickedUpAtSimSeconds;
		R.CompletedAtSimSeconds = S.CompletedAtSimSeconds;
		R.FailedAtSimSeconds = S.FailedAtSimSeconds;
		R.FailureReason = S.FailureReason;
		SaveObject->ContractRecords.Add(R);
	}

	return true;
}

bool UPersistenceSubsystem::ApplyFrom(const UCourier404SaveGame* SaveObject)
{
	UContractServiceSubsystem* ContractsSub = Contracts.Get();
	ULifeSubsystem* LifeSub = Life.Get();
	if (!SaveObject || !ContractsSub || !LifeSub)
	{
		return false;
	}
	if (SaveObject->SaveVersion != UCourier404SaveGame::CurrentVersion)
	{
		UE_LOG(LogCourier404, Error, TEXT("Save version %d incompatible with %d; refusing load"),
			SaveObject->SaveVersion, UCourier404SaveGame::CurrentVersion);
		return false;
	}

	FCourier404LifeService& DstLife = LifeSub->GetLife();
	DstLife.GetWallet().SetBalance(SaveObject->Money);

	if (FCourier404SimClock* Clock = DstLife.GetClock())
	{
		Clock->Restore(SaveObject->DayIndex, SaveObject->HourOfDay);
	}

	DstLife.GetNeeds().Restore(SaveObject->Hunger01, SaveObject->Fatigue01,
		SaveObject->Health01, SaveObject->TimeStarvingHours);

	// Contract instances restore exactly as saved: completed jobs never re-pay.
	FCourier404ContractService& Domain = ContractsSub->GetDomain();
	for (const FCourier404ContractRecord& R : SaveObject->ContractRecords)
	{
		FContractRuntimeState S;
		S.InstanceId = R.InstanceId;
		S.ContractId = R.ContractId;
		S.Status = R.Status;
		S.AcceptedAtSimSeconds = R.AcceptedAtSimSeconds;
		S.PickedUpAtSimSeconds = R.PickedUpAtSimSeconds;
		S.CompletedAtSimSeconds = R.CompletedAtSimSeconds;
		S.FailedAtSimSeconds = R.FailedAtSimSeconds;
		S.FailureReason = R.FailureReason;
		Domain.RestoreInstance(S);
	}

	return true;
}

bool UPersistenceSubsystem::SaveToSlot(const TCHAR* SlotName)
{
	UCourier404SaveGame* SaveObject = Cast<UCourier404SaveGame>(
		UGameplayStatics::CreateSaveGameObject(UCourier404SaveGame::StaticClass()));
	if (!CaptureTo(SaveObject))
	{
		return false;
	}
	return UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, 0);
}

bool UPersistenceSubsystem::LoadFromSlot(const TCHAR* SlotName)
{
	UCourier404SaveGame* Loaded = Cast<UCourier404SaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	return Loaded ? ApplyFrom(Loaded) : false;
}
