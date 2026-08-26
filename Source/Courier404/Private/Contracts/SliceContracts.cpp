#include "Contracts/SliceContracts.h"

namespace Courier404SliceContracts
{
	void RegisterDefaults(FCourier404ContractService& Service)
	{
		FContractDefinition Normal;
		Normal.ContractId = TEXT("Job.Courier01");
		Normal.Category = EContractCategory::Normal;
		Normal.PickupPointId = TEXT("Point.Store");
		Normal.DropoffPointId = TEXT("Point.DropA");
		Normal.Reward = 120;
		Normal.TimeLimitSeconds = 3600.f; // generous: one full day-night cycle
		Normal.CargoId = TEXT("Cargo.Parcel");
		TArray<FString> Errors;
		if (!Service.RegisterDefinition(Normal, Errors))
		{
			// Already registered (idempotent re-init): acceptable.
			return;
		}

		FContractDefinition Anon;
		Anon.ContractId = TEXT("Job.NightDrop");
		Anon.Category = EContractCategory::Anonymous;
		Anon.PickupPointId = TEXT("Point.Locker");
		Anon.DropoffPointId = TEXT("Point.RiskyDrop");
		Anon.Reward = 900;
		Anon.TimeLimitSeconds = 1800.f;
		Anon.CargoId = TEXT("Cargo.SealedBag");
		Anon.RiskLevel = 3;
		Anon.Rules = { TEXT("Rule.AnonymousSender"), TEXT("Rule.PoliceRisk") };
		Service.RegisterDefinition(Anon, Errors);
	}
}
