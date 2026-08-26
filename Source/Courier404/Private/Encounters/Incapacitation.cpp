#include "Encounters/Incapacitation.h"

FCourier404DeathRecovery FCourier404Incapacitation::Recover(
	ECourier404DeathSource Source,
	FCourier404Wallet& Wallet,
	FCourier404Needs& Needs,
	FCourier404SimClock& Clock,
	const FHooks& Hooks)
{
	FCourier404DeathRecovery R;
	R.Source = Source;

	// Medical bill, always affordable by construction.
	const int32 Balance = Wallet.GetBalance();
	R.MedicalCost = FMath::Min(MedicalCostMax, Balance);
	Wallet.TrySpend(R.MedicalCost);

	// Wake at home next morning.
	Clock.SleepTo(0.f);            // normalize remainder of the night
	Clock.AdvanceHours(24.f);      // skip a full cycle for recovery
	Clock.Restore(Clock.GetDayIndex(), 8.f);
	R.WakeDayIndex = Clock.GetDayIndex();
	R.WakeHour = Clock.GetHourOfDay();

	// Body recovers, but not fully.
	Needs.Restore(/*hunger*/ 0.55f, /*fatigue*/ 0.85f, /*health*/ 0.5f, /*starving*/ 0.f);

	// Any active work dies with you.
	if (Hooks.HasActiveContract && Hooks.HasActiveContract())
	{
		if (Hooks.FailActiveContracts)
		{
			Hooks.FailActiveContracts(TEXT("Player.Incapacitated"));
			R.bFailedActiveContract = true;
		}
	}

	return R;
}