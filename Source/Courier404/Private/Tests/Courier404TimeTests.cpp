#include "Contracts/ContractDomain.h"
#include "Time/SimClock.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ClockAdvancement,
	"Courier404.Time.AdvancementAndDayRollover",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ClockAdvancement::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock;
	TestEqual(TEXT("starts day 1"), Clock.GetDayIndex(), 1);
	TestEqual(TEXT("starts at 08:00"), Clock.GetHourOfDay(), 8.f);

	// Zero/negative advance is a no-op.
	Clock.AdvanceHours(0.f);
	Clock.AdvanceHours(-3.f);
	TestEqual(TEXT("no-op advance keeps hour"), Clock.GetHourOfDay(), 8.f);
	TestEqual(TEXT("no-op advance keeps day"), Clock.GetDayIndex(), 1);

	// Cross midnight exactly once: 08:00 + 20h -> next day 04:00.
	const int32 Rollovers = Clock.AdvanceHours(20.f);
	TestEqual(TEXT("one rollover"), Rollovers, 1);
	TestEqual(TEXT("day incremented"), Clock.GetDayIndex(), 2);
	TestEqual(TEXT("hour wrapped"), Clock.GetHourOfDay(), 4.f);

	// Multi-day jump.
	Clock.AdvanceHours(72.f); // 3 full days
	TestEqual(TEXT("three more rollovers land on same hour"), Clock.GetHourOfDay(), 4.f);
	TestEqual(TEXT("day index advanced by three"), Clock.GetDayIndex(), 5);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ClockPhases,
	"Courier404.Time.PhaseBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ClockPhases::RunTest(const FString& Parameters)
{
	struct FPhaseCase
	{
		float Hour;
		ECourier404DayPhase Phase;
		bool bNight;
	};
	const FPhaseCase Cases[] = {
		{0.f, ECourier404DayPhase::Night, true},
		{5.9f, ECourier404DayPhase::Night, true},
		{6.f, ECourier404DayPhase::Morning, false},
		{10.9f, ECourier404DayPhase::Morning, false},
		{11.f, ECourier404DayPhase::Day, false},
		{17.9f, ECourier404DayPhase::Day, false},
		{18.f, ECourier404DayPhase::Evening, false},
		{21.9f, ECourier404DayPhase::Evening, false},
		{22.f, ECourier404DayPhase::Night, true},
	};

	for (const FPhaseCase& Case : Cases)
	{
		FCourier404SimClock Clock; // starts 08:00
		const float Offset = FMath::Fmod(Case.Hour - 8.f + 24.f, 24.f);
		Clock.AdvanceHours(Offset);
		TestEqual(FString::Printf(TEXT("phase at %.1fh"), Case.Hour), Clock.GetPhase(), Case.Phase);
		TestEqual(FString::Printf(TEXT("night flag at %.1fh"), Case.Hour), static_cast<int>(Clock.IsNight()), static_cast<int>(Case.bNight));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ClockSleep,
	"Courier404.Time.SleepJump",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ClockSleep::RunTest(const FString& Parameters)
{
	FCourier404SimClock Clock;
	Clock.AdvanceHours(15.f); // 23:00

	const float Jumped = Clock.SleepTo(7.f); // sleep to 07:00
	TestEqual(TEXT("sleep jumped eight hours"), Jumped, 8.f);
	TestEqual(TEXT("woke at 07:00"), Clock.GetHourOfDay(), 7.f);
	TestEqual(TEXT("rolled into next day"), Clock.GetDayIndex(), 2);

	// Sleeping backwards wraps forward to the next occurrence.
	Clock.AdvanceHours(2.f); // 09:00
	const float Backwards = Clock.SleepTo(7.f);
	TestEqual(TEXT("backwards target wraps to tomorrow"), Backwards, 22.f);
	TestEqual(TEXT("day advanced again"), Clock.GetDayIndex(), 3);

	// Sleeping to the current hour is a no-op.
	const float Noop = Clock.SleepTo(Clock.GetHourOfDay());
	TestEqual(TEXT("noop sleep returns zero"), Noop, 0.f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ClockWindows,
	"Courier404.Time.AvailabilityWindows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ClockWindows::RunTest(const FString& Parameters)
{
	// Non-wrapping window 9-17.
	TestTrue(TEXT("inside normal window"), FCourier404SimClock::IsHourInWindow(12.f, 9.f, 17.f));
	TestFalse(TEXT("before normal window"), FCourier404SimClock::IsHourInWindow(8.9f, 9.f, 17.f));
	TestFalse(TEXT("after normal window"), FCourier404SimClock::IsHourInWindow(17.f, 9.f, 17.f));

	// Wrapping window 20-2 (evening/night offer).
	TestTrue(TEXT("inside wrap window late"), FCourier404SimClock::IsHourInWindow(21.f, 20.f, 2.f));
	TestTrue(TEXT("inside wrap window early"), FCourier404SimClock::IsHourInWindow(1.f, 20.f, 2.f));
	TestFalse(TEXT("outside wrap window morning"), FCourier404SimClock::IsHourInWindow(3.f, 20.f, 2.f));
	TestFalse(TEXT("outside wrap window afternoon"), FCourier404SimClock::IsHourInWindow(19.9f, 20.f, 2.f));

	// Integration with the clock itself.
	FCourier404SimClock Clock; // 08:00
	Clock.AdvanceHours(14.f); // 22:00
	TestTrue(TEXT("clock feeds windows"), FCourier404SimClock::IsHourInWindow(Clock.GetHourOfDay(), 20.f, 2.f));

	return true;
}

#endif
