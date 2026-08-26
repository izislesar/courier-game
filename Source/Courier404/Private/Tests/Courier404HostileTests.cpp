#include "Courier404CargoTests.h"
#include "Misc/AutomationTest.h"
#include "Encounters/HostileEncounter.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404HostileFlee,
	"Courier404.Hostile.WarnFleeDisengageNeverStuck",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404HostileFlee::RunTest(const FString& Parameters)
{
	// Single attacker: fleeing always works, no injury.
	FCourier404HostileEncounter Solo;
	Solo.Start(1);
	TestEqual(TEXT("starts at warning"), static_cast<int>(Solo.GetSnapshot().State),
		static_cast<int>(ECourier404HostileState::Warn));

	Solo.Step(ECourier404PlayerAction::Flee, 400.f, false, 100);
	TestTrue(TEXT("escaped single attacker"), Solo.IsOver());
	TestEqual(TEXT("unharmed"), Solo.GetSnapshot().Health01, 1.f);
	TestFalse(TEXT("nothing stolen"), Solo.GetSnapshot().bRobbed);

	// Stuck-guard: pursuit breaks after the timeout even with None input.
	FCourier404HostileEncounter Chase;
	Chase.Start(2);
	for (int32 Step = 0; Step < 30 && !Chase.IsOver(); ++Step)
	{
		Chase.Step(ECourier404PlayerAction::None, 600.f, false, 0); // mid distance
	}
	TestTrue(TEXT("pursuit auto-disengages"), Chase.IsOver());
	TestTrue(TEXT("marked as escape"), Chase.GetSnapshot().bEscaped);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404HostileGroup,
	"Courier404.Hostile.MultipleAttackersInjureRobAndCanKill",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404HostileGroup::RunTest(const FString& Parameters)
{
	FCourier404HostileEncounter Gang;
	Gang.Start(3); // group cuts off naive escapes

	// Naive flee without sprinting gets caught and hurts.
	Gang.Step(ECourier404PlayerAction::Flee, 400.f, /*sprint*/false, 300);
	TestTrue(TEXT("caught while turning"), Gang.GetSnapshot().Health01 < 1.f);

	// Keep standing still at melee range: hits accumulate until robbery.
	int32 Steps = 0;
	while (!Gang.GetSnapshot().bRobbed && Steps < 20 && !Gang.IsOver())
	{
		Gang.Step(ECourier404PlayerAction::None, 100.f, false, 500);
		++Steps;
	}
	TestTrue(TEXT("robbed when losing"), Gang.GetSnapshot().bRobbed);
	TestEqual(TEXT("robbery capped"), Gang.GetSnapshot().MoneyStolen, FCourier404HostileEncounter::RobberyMax);

	// Lethal path: a fresh victim at melee range with no wallet still dies eventually.
	FCourier404HostileEncounter Lethal;
	Lethal.Start(3);
	int32 Guard = 0;
	while (!Lethal.GetSnapshot().bDead && Guard < 30 && !Lethal.IsOver())
	{
		Lethal.Step(ECourier404PlayerAction::None, 100.f, false, 0);
		++Guard;
	}
	TestTrue(TEXT("severe outcome kills"), Lethal.GetSnapshot().bDead);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404HostileShove,
	"Courier404.Hostile.ShoveStaggersWithoutDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404HostileShove::RunTest(const FString& Parameters)
{
	FCourier404HostileEncounter E;
	E.Start(1);
	E.Step(ECourier404PlayerAction::None, 400.f, false, 0); // pursue begins
	const float Before = E.GetSnapshot().Health01;

	E.Step(ECourier404PlayerAction::Shove, 200.f, false, 0);
	TestEqual(TEXT("shove resets to warning"), static_cast<int>(E.GetSnapshot().State),
		static_cast<int>(ECourier404HostileState::Warn));
	TestEqual(TEXT("no damage from shoving"), E.GetSnapshot().Health01, Before);

	return true;
}

#endif
