#pragma once

#include "CoreMinimal.h"
#include "Economy/Wallet.h"
#include "Needs/NeedsModel.h"
#include "Time/SimClock.h"
#include "Relationship/Relationship.h"

/**
 * Bounded life service for the pre-prod single player: wallet + needs +
 * clock-driven rest. Presentation and world interactables emit commands here.
 *
 * Plain deterministic C++; the owning subsystem supplies the clock.
 */
class COURIER404_API FCourier404LifeService
{
public:
	static constexpr int32 DefaultMealPrice = 15;

	explicit FCourier404LifeService(FCourier404SimClock* InClock = nullptr)
		: Clock(InClock) {}

	void BindClock(FCourier404SimClock* InClock) { Clock = InClock; }

	FCourier404Wallet& GetWallet() { return Wallet; }
	const FCourier404Wallet& GetWallet() const { return Wallet; }
	FCourier404SimClock* GetClock() { return Clock; }
	const FCourier404SimClock* GetClock() const { return Clock; }
	FCourier404Needs& GetNeeds() { return Needs; }
	const FCourier404Needs& GetNeeds() const { return Needs; }

	FCourier404Relationship& GetRelationship() { return Relationship; }
	const FCourier404Relationship& GetRelationship() const { return Relationship; }

	/**
	 * Buys and eats one meal at the given price. Refuses unaffordable/invalid
	 * prices without touching any state. Advances needs by elapsed time first.
	 */
	bool Eat(float ElapsedHoursSinceLastVisit, int32 Price = DefaultMealPrice);

	/**
	 * Sleeps until TargetHour (next occurrence). Advances the clock, applies
	 * recovery for the slept hours and returns hours slept (<=0 refused).
	 */
	float SleepTo(float TargetHour);

	/** Passive passage of time (called by tick/integration). */
	void Advance(float Hours) { Needs.Advance(Hours); }

private:
	FCourier404SimClock* Clock = nullptr;
	FCourier404Wallet Wallet;
	FCourier404Needs Needs;
	FCourier404Relationship Relationship;
};
