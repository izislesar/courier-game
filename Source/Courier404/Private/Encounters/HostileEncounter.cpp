#include "Encounters/HostileEncounter.h"

void FCourier404HostileEncounter::Start(int32 AttackerCount)
{
	Snapshot = FCourier404HostileSnapshot();
	Snapshot.Attackers = FMath::Max(1, AttackerCount);
	Snapshot.State = ECourier404HostileState::Warn;
}

void FCourier404HostileEncounter::ApplyAttack(int32 WalletBalance)
{
	Snapshot.Health01 = FMath::Max(0.f, Snapshot.Health01 - DamagePerHit);

	// They rob you once you are clearly losing.
	if (!Snapshot.bRobbed && Snapshot.Health01 < RobberyHealthThreshold && WalletBalance > 0)
	{
		Snapshot.MoneyStolen = FMath::Min(RobberyMax, WalletBalance);
		Snapshot.bRobbed = true;
	}

	if (Snapshot.Health01 <= 0.f)
	{
		Snapshot.bDead = true;
		Snapshot.State = ECourier404HostileState::Disengaged;
	}
}

void FCourier404HostileEncounter::Step(
	ECourier404PlayerAction Action, float DistanceCm, bool bPlayerSprinting, int32 WalletBalance)
{
	if (IsOver() || Snapshot.bDead)
	{
		return;
	}

	switch (Action)
	{
	case ECourier404PlayerAction::Flee:
	{
		// One attacker: sprinting always breaks away; walking works too.
		// Multiple attackers cut off naive escapes unless sprinting.
		const bool bEscape = Snapshot.Attackers <= 1 || bPlayerSprinting;
		if (bEscape)
		{
			Snapshot.State = ECourier404HostileState::Disengaged;
			Snapshot.bEscaped = true;
			return;
		}
		ApplyAttack(WalletBalance); // caught while turning
		break;
	}
	case ECourier404PlayerAction::Shove:
		// Buys space: attacker falls back to warning; no damage either way.
		Snapshot.State = ECourier404HostileState::Warn;
		return;
	case ECourier404PlayerAction::Strike:
		// Striking into a group guarantees taking a hit.
		if (Snapshot.Attackers >= 2)
		{
			ApplyAttack(WalletBalance);
			if (Snapshot.bDead)
			{
				return;
			}
		}
		Snapshot.State = ECourier404HostileState::Attack;
		break;
	case ECourier404PlayerAction::None:
	default:
		break;
	}

	// Distance logic.
	if (DistanceCm > BreakDistance)
	{
		Snapshot.State = ECourier404HostileState::Disengaged;
		Snapshot.bEscaped = true;
		return;
	}

	if (DistanceCm < 150.f)
	{
		Snapshot.State = ECourier404HostileState::Attack;
		ApplyAttack(WalletBalance);
		if (Snapshot.bDead || Snapshot.State == ECourier404HostileState::Disengaged)
		{
			return;
		}
	}
	else
	{
		Snapshot.State = ECourier404HostileState::Pursue;
		Snapshot.PursueElapsed += 1.f; // step granularity
		if (Snapshot.PursueElapsed > MaxPursueSeconds)
		{
			// Stuck-guard: they give up rather than chase forever.
			Snapshot.State = ECourier404HostileState::Disengaged;
			Snapshot.bEscaped = true;
			return;
		}
	}
}