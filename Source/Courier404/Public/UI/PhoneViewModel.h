#pragma once

#include "CoreMinimal.h"
#include "Contracts/ContractDomain.h"
#include "Time/SimClock.h"

/**
 * Phone presentation model. Reads contract/clock domain state and exposes
 * commands; owns no contract truth. Presentation (HUD) renders this snapshot.
 */
class COURIER404_API FCourier404PhoneViewModel
{
public:
	enum class EScreen : uint8
	{
		Jobs,
		ActiveJob,
		Messages
	};

	struct FOffer
	{
		FName ContractId;
		EContractCategory Category;
		int32 Reward = 0;
		FName PickupPointId;
		FName DropoffPointId;
		bool bNightOnly = false;
	};

	void Bind(FCourier404ContractService* InContracts, const FCourier404SimClock* InClock);

	void SetScreen(EScreen NewScreen) { Screen = NewScreen; }
	EScreen GetScreen() const { return Screen; }
	void CycleScreen(int32 Direction);

	/** Rebuilds the offer list: registered contracts not accepted/completed; anonymous only at night. */
	void Refresh();

	const TArray<FOffer>& GetOffers() const { return Offers; }
	int32 GetSelectedIndex() const { return SelectedIndex; }
	void MoveSelection(int32 Delta);

	/** Accepts the selected offer; returns instance id (none on failure). */
	FName AcceptSelected();

	/** Rejects the selected offer for this session. */
	bool RejectSelected();

	FName GetActiveInstanceId() const { return ActiveInstanceId; }

	/** One-line objective for the active job, e.g. "Pickup Point.Store -> Point.DropA (120)". */
	FString GetObjectiveLine() const;

	const TArray<FString>& GetMessages() const { return Messages; }

private:
	FCourier404ContractService* Contracts = nullptr;
	const FCourier404SimClock* Clock = nullptr;

	EScreen Screen = EScreen::Jobs;
	TArray<FOffer> Offers;
	int32 SelectedIndex = 0;

	TSet<FName> RejectedThisSession;
	TSet<FName> AcceptedThisSession;
	FName ActiveInstanceId;
	TArray<FString> Messages;
};
