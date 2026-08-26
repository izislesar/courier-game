#include "UI/PhoneViewModel.h"
#include "Courier404.h"

void FCourier404PhoneViewModel::Bind(FCourier404ContractService* InContracts, const FCourier404SimClock* InClock)
{
	Contracts = InContracts;
	Clock = InClock;
}

void FCourier404PhoneViewModel::CycleScreen(int32 Direction)
{
	const int32 Count = 3;
	Screen = static_cast<EScreen>((static_cast<int32>(Screen) + Direction + Count) % Count);
}

void FCourier404PhoneViewModel::Refresh()
{
	Offers.Reset();
	if (!Contracts)
	{
		return;
	}

	const bool bNightWindow = Clock ? Clock->IsNight() : true; // no clock: show all

	// Deterministic order by contract id.
	TArray<FName> Ids;
	for (const TPair<FName, FContractDefinition>& Pair : Contracts->GetDefinitions())
	{
		Ids.Add(Pair.Key);
	}
	Ids.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });

	bool bHasOpenInstance = false;
	if (Contracts)
	{
		for (const TPair<FName, FContractRuntimeState>& Pair : Contracts->GetInstances())
		{
			const EContractStatus S = Pair.Value.Status;
			if ((S == EContractStatus::Accepted || S == EContractStatus::PickedUp))
			{
				bHasOpenInstance = true;
				break;
			}
		}
	}

	for (const FName& Id : Ids)
	{
		if (RejectedThisSession.Contains(Id) || AcceptedThisSession.Contains(Id))
		{
			continue;
		}
		if (bHasOpenInstance)
		{
			continue; // one active job at a time in the slice
		}
		const FContractDefinition* Def = Contracts->FindDefinition(Id);
		if (!Def || (Def->Category != EContractCategory::Normal && Def->RiskLevel <= 0))
		{
			continue;
		}

		if (Def->Category == EContractCategory::Anonymous && !bNightWindow)
		{
			continue; // anonymous offers surface only in the evening/night
		}

		FOffer Entry;
		Entry.ContractId = Def->ContractId;
		Entry.Category = Def->Category;
		Entry.Reward = Def->Reward;
		Entry.PickupPointId = Def->PickupPointId;
		Entry.DropoffPointId = Def->DropoffPointId;
		Entry.bNightOnly = Def->Category == EContractCategory::Anonymous;
		Offers.Add(Entry);
	}

	SelectedIndex = FMath::Clamp(SelectedIndex, 0, FMath::Max(0, Offers.Num() - 1));
}

void FCourier404PhoneViewModel::MoveSelection(int32 Delta)
{
	if (Offers.IsEmpty())
	{
		SelectedIndex = 0;
		return;
	}
	SelectedIndex = FMath::Clamp(SelectedIndex + Delta, 0, Offers.Num() - 1);
}

FName FCourier404PhoneViewModel::AcceptSelected()
{
	if (!Contracts || Offers.IsValidIndex(SelectedIndex) == false)
	{
		return NAME_None;
	}

	const FOffer& Offer = Offers[SelectedIndex];
	const float Now = Clock ? Clock->GetHourOfDay() * FCourier404SimClock::SecondsPerHour : 0.f;
	const FName Instance = Contracts->Accept(Offer.ContractId, Now);
	if (!Instance.IsNone())
	{
		ActiveInstanceId = Instance;
		AcceptedThisSession.Add(Offer.ContractId);
		Messages.Add(FString::Printf(TEXT("Accepted %s (%d cr)."), *Offer.ContractId.ToString(), Offer.Reward));
	}
	return Instance;
}

bool FCourier404PhoneViewModel::RejectSelected()
{
	if (!Offers.IsValidIndex(SelectedIndex))
	{
		return false;
	}
	RejectedThisSession.Add(Offers[SelectedIndex].ContractId);
	Messages.Add(FString::Printf(TEXT("Declined %s."), *Offers[SelectedIndex].ContractId.ToString()));
	Refresh();
	return true;
}

FString FCourier404PhoneViewModel::GetObjectiveLine() const
{
	if (!Contracts)
	{
		return TEXT("No active job");
	}

	// Derive from service truth: prefer an open job; otherwise show the most
	// recent finished job as feedback.
	const FContractRuntimeState* State = nullptr;
	float BestFinish = -TNumericLimits<float>::Max();

	for (const TPair<FName, FContractRuntimeState>& Pair : Contracts->GetInstances())
	{
		const FContractRuntimeState& C = Pair.Value;
		const bool bOpen = C.Status == EContractStatus::Accepted || C.Status == EContractStatus::PickedUp;
		const float FinishTime = FMath::Max(C.CompletedAtSimSeconds, C.FailedAtSimSeconds);

		if (bOpen)
		{
			// Earliest open job wins; terminal candidates never beat it.
			if (!State || State->Status == EContractStatus::Completed || State->Status == EContractStatus::Failed ||
				C.AcceptedAtSimSeconds < State->AcceptedAtSimSeconds)
			{
				State = &C;
			}
		}
		else if ((C.Status == EContractStatus::Completed || C.Status == EContractStatus::Failed) &&
			(!State || (State->Status != EContractStatus::Accepted && State->Status != EContractStatus::PickedUp)) &&
			FinishTime >= BestFinish)
		{
			State = &C;
			BestFinish = FinishTime;
		}
	}
	if (!State)
	{
		return TEXT("No active job");
	}

	const FContractDefinition* Def = Contracts->FindDefinition(State->ContractId);
	if (!Def)
	{
		return TEXT("No active job");
	}

	switch (State->Status)
	{
	case EContractStatus::Accepted:
		return FString::Printf(TEXT("Pick up %s -> deliver to %s (%d cr)"),
			*Def->PickupPointId.ToString(), *Def->DropoffPointId.ToString(), Def->Reward);
	case EContractStatus::PickedUp:
		return FString::Printf(TEXT("Deliver to %s (%d cr)"), *Def->DropoffPointId.ToString(), Def->Reward);
	case EContractStatus::Completed:
		return FString::Printf(TEXT("Delivered (+%d cr)"), Def->Reward);
	case EContractStatus::Failed:
		return FString::Printf(TEXT("Job failed (%s)"), *State->FailureReason.ToString());
	default:
		return TEXT("No active job");
	}
}