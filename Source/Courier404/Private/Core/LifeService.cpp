#include "Core/LifeService.h"

bool FCourier404LifeService::Eat(float ElapsedHoursSinceLastVisit, int32 Price)
{
	if (Price <= 0 || ElapsedHoursSinceLastVisit < 0.f)
	{
		return false;
	}

	// Time passes before the transaction; hunger may have degraded further.
	Needs.Advance(ElapsedHoursSinceLastVisit);

	if (!Wallet.TrySpend(Price))
	{
		return false; // cannot afford: no partial state changes
	}

	Needs.Eat();
	return true;
}

float FCourier404LifeService::SleepTo(float TargetHour)
{
	if (!Clock)
	{
		return 0.f;
	}

	// Pre-sleep fatigue decay up to lights-out.
	// (The clock jump itself covers the remaining elapsed time.)
	const float Jumped = Clock->SleepTo(TargetHour);
	if (Jumped <= 0.f)
	{
		return 0.f;
	}

	// Recovery from rest outweighs decay across the slept window.
	Needs.Sleep(Jumped);
	return Jumped;
}
