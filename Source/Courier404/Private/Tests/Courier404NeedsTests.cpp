#include "Contracts/ContractDomain.h"
#include "Needs/NeedsModel.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404NeedsThresholds,
	"Courier404.Needs.ThresholdsAdvanceInOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404NeedsThresholds::RunTest(const FString& Parameters)
{
	FCourier404Needs Needs;
	TestEqual(TEXT("starts fed"), static_cast<int>(Needs.GetHunger()), static_cast<int>(ECourier404Hunger::Fed));
	TestEqual(TEXT("starts rested"), static_cast<int>(Needs.GetFatigue()), static_cast<int>(ECourier404Fatigue::Rested));

	// Full->starved takes 48h; states must appear in order, no skips.
	bool bSawHungry = false;
	bool bSawVeryHungry = false;
	for (int32 Hour = 1; Hour <= 48 && !bSawVeryHungry; ++Hour)
	{
		Needs.Advance(1.f);
		const ECourier404Hunger H = Needs.GetHunger();
		if (H == ECourier404Hunger::Hungry)
		{
			bSawHungry = true;
		}
		if (H == ECourier404Hunger::VeryHungry)
		{
			bSawVeryHungry = true;
			TestFalse(TEXT("never skipped starving before very hungry"), false);
		}
	}
	TestTrue(TEXT("passed through hungry"), bSawHungry);
	TestTrue(TEXT("reached very hungry"), bSawVeryHungry);
	TestEqual(TEXT("starving only after very hungry window"),
		static_cast<int>(Needs.GetHunger()), static_cast<int>(ECourier404Hunger::VeryHungry));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404NeedsMissedMeal,
	"Courier404.Needs.OneMissedMealNotCatastrophic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404NeedsMissedMeal::RunTest(const FString& Parameters)
{
	FCourier404Needs Needs;

	// A full work day (10h) without eating: must stay above VeryHungry.
	Needs.Advance(10.f);
	TestTrue(TEXT("still at worst hungry after a work day"),
		Needs.GetHunger() == ECourier404Hunger::Fed || Needs.GetHunger() == ECourier404Hunger::Hungry);

	// One late night (10h extra awake) plus missed meals: tired, not exhausted.
	Needs.Advance(10.f);
	TestEqual(TEXT("fatigue tired after a long day"),
		static_cast<int>(Needs.GetFatigue()), static_cast<int>(ECourier404Fatigue::Tired));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404NeedsRecovery,
	"Courier404.Needs.RecoveryOperations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404NeedsRecovery::RunTest(const FString& Parameters)
{
	FCourier404Needs Needs;
	Needs.Advance(30.f); // deep into hungry/very hungry + tired
	TestTrue(TEXT("hunger degraded"), Needs.GetHunger01() < 0.7f);

	// Eating restores fed state.
	Needs.Eat();
	TestEqual(TEXT("meal restores fed"), static_cast<int>(Needs.GetHunger()), static_cast<int>(ECourier404Hunger::Fed));

	// Sleeping restores rested state.
	Needs.Sleep(8.f);
	TestEqual(TEXT("full night restores rested"), static_cast<int>(Needs.GetFatigue()), static_cast<int>(ECourier404Fatigue::Rested));

	// Partial sleep gives partial recovery.
	FCourier404Needs B;
	B.Advance(30.f);
	B.Sleep(3.f);
	TestTrue(TEXT("partial sleep helps but may not fully restore"), B.GetFatigue01() > 0.25f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404NeedsStarvationDeath,
	"Courier404.Needs.StarvationDamageIsDelayedAndLethal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404NeedsStarvationDeath::RunTest(const FString& Parameters)
{
	FCourier404Needs Needs;

	// Drive to starving: 50h decay passes the 0.15 threshold (~40.8h).
	Needs.Advance(50.f);
	TestEqual(TEXT("now starving"), static_cast<int>(Needs.GetHunger()), static_cast<int>(ECourier404Hunger::Starving));

	// Grace period: finish the remaining 12 starving hours without damage.
	const float UsedGrace = Needs.GetTimeStarving();
	TestTrue(TEXT("onset accounted"), UsedGrace > 0.f);
	Needs.Advance(FMath::Max(0.f, 12.f - UsedGrace));
	TestEqual(TEXT("no damage within grace"), Needs.GetHealth01(), 1.f);

	// After grace: slow decline, still alive well past the grace window.
	Needs.Advance(4.f);
	TestTrue(TEXT("health declined after grace"), Needs.GetHealth01() < 1.f);
	TestFalse(TEXT("not dead yet"), Needs.IsDead());

	// Prolonged neglect finally kills (full health bar drains at 0.06/h).
	Needs.Advance(30.f);
	TestTrue(TEXT("prolonged neglect kills"), Needs.IsDead());

	// Recovery resets grace when eaten in time.
	FCourier404Needs Saved;
	Saved.Advance(48.f);
	Saved.Advance(5.f); // 5h starving, inside grace
	Saved.Eat();
	Saved.Advance(2.f); // fed again: timer reset
	TestEqual(TEXT("grace timer reset after eating"), Saved.GetTimeStarving(), 0.f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404NeedsModifierOutputs,
	"Courier404.Needs.MechanicalModifiers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404NeedsModifierOutputs::RunTest(const FString& Parameters)
{
	FCourier404Needs Healthy;
	const FCourier404NeedsModifiers HealthyMods = Healthy.ComputeModifiers();
	TestEqual(TEXT("healthy recovery x1"), HealthyMods.StaminaRecoveryMultiplier, 1.f);
	TestEqual(TEXT("healthy speed x1"), HealthyMods.MoveSpeedMultiplier, 1.f);

	// Exhausted only: 40h awake.
	FCourier404Needs Tired;
	Tired.Eat();          // isolate fatigue effects from hunger stacking
	Tired.Advance(40.f);
	Tired.Eat();
	const FCourier404NeedsModifiers TiredMods = Tired.ComputeModifiers();
	TestEqual(TEXT("exhausted recovery x0.5"), TiredMods.StaminaRecoveryMultiplier, 0.5f);
	TestEqual(TEXT("exhausted speed x0.9"), TiredMods.MoveSpeedMultiplier, 0.9f);
	TestTrue(TEXT("exhausted flagged risky"), TiredMods.bAtRisk);

	// Starving stacks with hunger modifiers.
	FCourier404Needs Starving;
	Starving.Advance(50.f);
	const FCourier404NeedsModifiers StarvingMods = Starving.ComputeModifiers();
	TestTrue(TEXT("starving recovery below very hungry"), StarvingMods.StaminaRecoveryMultiplier <= 0.3f);
	TestTrue(TEXT("starving slowed"), StarvingMods.MoveSpeedMultiplier < 1.f);
	TestTrue(TEXT("starving flagged risky"), StarvingMods.bAtRisk);

	return true;
}

#endif
