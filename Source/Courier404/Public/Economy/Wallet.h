#pragma once

#include "CoreMinimal.h"

/**
 * Minimal money balance for the slice. Never negative; every mutation guarded.
 */
class COURIER404_API FCourier404Wallet
{
public:
	int32 GetBalance() const { return Balance; }

	void Add(int32 Amount)
	{
		if (Amount > 0)
		{
			Balance += Amount;
		}
	}

	/** Returns false and changes nothing when Amount is invalid or unaffordable. */
	bool TrySpend(int32 Amount)
	{
		if (Amount <= 0 || Amount > Balance)
		{
			return false;
		}
		Balance -= Amount;
		return true;
	}

	/** Persistence: exact balance restore (negative rejected). */
	void SetBalance(int32 Amount) { if (Amount >= 0) { Balance = Amount; } }

private:
	int32 Balance = 0;
};
