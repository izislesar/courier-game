#pragma once

#include "CoreMinimal.h"

/**
 * Minimal hostile-encounter semantics for the slice: warning, pursue, attack,
 * disengage/flee. Deterministic; presentation drives steps from AI ticks.
 * Encounter can never get stuck: pursuit auto-breaks after a timeout.
 */
enum class ECourier404HostileState : uint8
{
	Idle,
	Warn,
	Pursue,
	Attack,
	Disengaged
};

enum class ECourier404PlayerAction : uint8
{
	None,
	Flee,
	Shove,
	Strike
};

struct COURIER404_API FCourier404HostileSnapshot
{
	ECourier404HostileState State = ECourier404HostileState::Idle;
	float Health01 = 1.f;
	int32 Attackers = 0;
	int32 MoneyStolen = 0;
	bool bRobbed = false;
	bool bDead = false;
	bool bEscaped = false;
	float PursueElapsed = 0.f;
};

class COURIER404_API FCourier404HostileEncounter
{
public:
	static constexpr float DamagePerHit = 0.15f;      // of health
	static constexpr float RobberyHealthThreshold = 0.45f; // below this they take your money too
	static constexpr int32 RobberyMax = 80;
	static constexpr float MaxPursueSeconds = 25.f;   // stuck-guard
	static constexpr float BreakDistance = 1200.f;    // cm

	/** Begins an encounter with N attackers at warning range. */
	void Start(int32 AttackerCount);

	/**
	 * Advances one resolution step.
	 * DistanceCm: current player-to-nearest-attacker distance.
	 */
	void Step(ECourier404PlayerAction Action, float DistanceCm, bool bPlayerSprinting, int32 WalletBalance);

	const FCourier404HostileSnapshot& GetSnapshot() const { return Snapshot; }

	/** True when the encounter ended by escape or by the stuck-guard. */
	bool IsOver() const
	{
		return Snapshot.State == ECourier404HostileState::Idle ||
			Snapshot.State == ECourier404HostileState::Disengaged;
	}

private:
	void ApplyAttack(int32 WalletBalance);

	FCourier404HostileSnapshot Snapshot;
};
