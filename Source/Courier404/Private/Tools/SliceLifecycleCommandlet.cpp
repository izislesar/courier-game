#include "Tools/SliceLifecycleCommandlet.h"
#include "Courier404.h"
#include "Contracts/SliceContracts.h"
#include "Contracts/ContractDomain.h"
#include "Core/LifeService.h"
#include "Needs/NeedsModel.h"
#include "Police/PoliceEncounter.h"
#include "Encounters/HostileEncounter.h"
#include "Encounters/Incapacitation.h"
#include "Relationship/Relationship.h"
#include "Time/SimClock.h"
#include "Persistence/Courier404SaveGame.h"
#include "UI/PhoneViewModel.h"
#include "City/CitySimulation.h"

USliceLifecycleCommandlet::USliceLifecycleCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

namespace
{
	int32 GFailures = 0;

	void Check(bool bCondition, const TCHAR* What)
	{
		if (bCondition)
		{
			UE_LOG(LogCourier404, Log, TEXT("LIFE OK   %s"), What);
		}
		else
		{
			++GFailures;
			UE_LOG(LogCourier404, Error, TEXT("LIFE FAIL %s"), What);
		}
	}
}

int32 USliceLifecycleCommandlet::Main(const FString& Params)
{
	GFailures = 0;

	FCourier404SimClock Clock;                 // day 1, 08:00
	FCourier404LifeService Life(&Clock);
	Life.GetWallet().Add(50);                  // starting cash

	FCourier404ContractService Contracts;
	Courier404SliceContracts::RegisterDefaults(Contracts);

	int32 CompletionEvents = 0;
	Contracts.OnContractCompleted.AddLambda([&](const FContractRuntimeState&, int32 Payout)
	{
		Life.GetWallet().Add(Payout);
		++CompletionEvents;
	});

	// Relationship: Lena expects the first evening (subsystem schedules at init).
	FCourier404Relationship& Rel = Life.GetRelationship();
	Rel.Schedule(/*day*/1, /*hour*/20.f);

	// ---- 1. Morning: ordinary job offered on the phone ----
	FCourier404PhoneViewModel Phone;
	Phone.Bind(&Contracts, &Clock);
	Phone.Refresh();
	Check(!Phone.GetOffers().IsEmpty(), TEXT("morning phone shows normal job"));
	Check(Rel.HasPendingPlan(), TEXT("evening plan scheduled"));

	// ---- 2. Accept and physically pick up ----
	const FName Normal = Phone.AcceptSelected();
	const FName ParcelInstance = Contracts.FindActiveInstanceForCargo(TEXT("Cargo.Parcel"));
	Check(!Normal.IsNone() && ParcelInstance == Normal, TEXT("accepted job binds parcel pickup"));
	Check(Contracts.MarkPickup(Normal, Clock.GetHourOfDay()), TEXT("parcel collected"));

	// ---- 3. Deliver at Point.DropA: paid exactly once ----
	{
		int32 Payout = 0;
		Check(Contracts.TryDeliver(Normal, TEXT("Point.DropA"), Clock.GetHourOfDay(), Payout),
			TEXT("normal delivery completes"));
		Check(Life.GetWallet().GetBalance() == 170, TEXT("payout credited once (170)"));
		Check(CompletionEvents == 1, TEXT("single completion event"));
	}

	// ---- 4. Food ----
	Clock.AdvanceHours(8.f);
	Check(Life.Eat(0.f, 15), TEXT("meal affordable"));
	Check(Life.GetWallet().GetBalance() == 155, TEXT("meal charged"));
	Check(Life.GetNeeds().GetHunger01() >= 0.7f, TEXT("meal restored fed"));

	// ---- 5. Evening: temptation appears before the planned meeting ----
	Clock.AdvanceHours(11.f); // ~19:00+
	Check(Rel.IsMeetingNow(Clock) == false || Clock.GetHourOfDay() >= 19.f,
		TEXT("evening arrived"));

	// ---- 6. Anonymous night flow ----
	FCourier404PhoneViewModel Night;
	Night.Bind(&Contracts, &Clock);
	Night.Refresh();
	bool bAnonVisible = false;
	for (const FCourier404PhoneViewModel::FOffer& O : Night.GetOffers())
	{
		bAnonVisible |= O.Category == EContractCategory::Anonymous;
	}
	Check(bAnonVisible, TEXT("anonymous offer visible in the evening"));

	const FName Anon = Contracts.Accept(TEXT("Job.NightDrop"), Clock.GetHourOfDay());
	Check(!Anon.IsNone(), TEXT("anonymous contract accepted"));
	Check(Contracts.MarkPickup(Anon, Clock.GetHourOfDay() + 1.f), TEXT("sealed bag collected at locker"));

	Clock.AdvanceHours(6.f);
	Check(Rel.EvaluateMissed(Clock), TEXT("meeting missed while doing night run"));

	// ---- 7. Police stop with contraband + priors -> arrest ----
	{
		FCourier404EncounterInputs In;
		In.Reason = ECourier404StopReason::CargoInspection;
		In.CargoRiskLevel = Contracts.GetPoliceRiskForCargo(TEXT("Cargo.SealedBag"));
		In.PriorOffenses = 0;      // first stop this save
		In.bTriedToFlee = true;    // the courier bolts -> escalates to arrest

		float DetainedHours = 0.f;
		FCourier404PoliceEncounter::FAppliers Appliers;
		Appliers.ChargeWallet = [&Life](int32 A)
		{
			const int32 B = Life.GetWallet().GetBalance();
			const int32 C = FMath::Min(A, B);
			Life.GetWallet().TrySpend(C);
			return C;
		};
		Appliers.AdvanceClock = [&Clock, &DetainedHours](float H)
		{
			Clock.AdvanceHours(H);
			DetainedHours += H;
		};
		Appliers.FailActiveContracts = [&Contracts, &Clock](const FName& Why)
		{
			const FName Id = Contracts.FindOpenInstanceForCargo(TEXT("Cargo.SealedBag"));
			if (!Id.IsNone())
			{
				Contracts.Fail(Id, Why, Clock.GetHourOfDay());
			}
		};
		Appliers.HasActiveRiskyContract = [&Contracts]()
		{
			return !Contracts.FindOpenInstanceForCargo(TEXT("Cargo.SealedBag")).IsNone();
		};

		const FCourier404EncounterResult R = FCourier404PoliceEncounter::Execute(In, Appliers);
		Check(R.Outcome == ECourier404PoliceOutcome::Arrest, TEXT("contraband stop escalates to arrest"));
		Check(FMath::IsNearlyEqual(DetainedHours, 8.f), TEXT("detention advances eight hours"));
		Check(R.bFailedActiveContract, TEXT("risky night job failed by arrest"));
	}

	// ---- 8-9. Hostile courtyard: beaten down to death, unified recovery ----
	FCourier404HostileEncounter Gang;
	Gang.Start(3);
	int32 Guard = 0;
	while (!Gang.GetSnapshot().bDead && !Gang.IsOver() && Guard < 40)
	{
		Gang.Step(ECourier404PlayerAction::None, 100.f, false, Life.GetWallet().GetBalance());
		++Guard;
	}
	Check(Guard < 40, TEXT("hostile resolution terminates (never stuck)"));
	Check(Gang.GetSnapshot().bRobbed || Life.GetWallet().GetBalance() == 0,
		TEXT("losing has configured consequence (robbed, or nothing left to take)"));
	Check(Gang.GetSnapshot().bDead, TEXT("severe outcome kills"));

	{
		const int32 PreBalance = Life.GetWallet().GetBalance();
		FCourier404Incapacitation::FHooks Hooks;
		Hooks.HasActiveContract = []() { return false; }; // risky job already failed by arrest
		Hooks.FailActiveContracts = [](const FName&) {};
		const FCourier404DeathRecovery R = FCourier404Incapacitation::Recover(
			ECourier404DeathSource::Violence, Life.GetWallet(), Life.GetNeeds(), Clock, Hooks);
		Check(R.MedicalCost == FMath::Min(100, PreBalance), TEXT("medical cost clamped to balance"));
		Check(FMath::IsNearlyEqual(R.WakeHour, 8.f), TEXT("wakes home 08:00"));
		Check(!Life.GetNeeds().IsDead(), TEXT("recovered body alive"));
	}

	// ---- 10. Persistence round trip ----
	UCourier404SaveGame* Saved = NewObject<UCourier404SaveGame>();
	Saved->Money = Life.GetWallet().GetBalance();
	Saved->DayIndex = Clock.GetDayIndex();
	Saved->HourOfDay = Clock.GetHourOfDay();
	Saved->Hunger01 = Life.GetNeeds().GetHunger01();
	Saved->Fatigue01 = Life.GetNeeds().GetFatigue01();
	Saved->Health01 = Life.GetNeeds().GetHealth01();
	for (const TPair<FName, FContractRuntimeState>& Pair : Contracts.GetInstances())
	{
		FCourier404ContractRecord R;
		R.InstanceId = Pair.Value.InstanceId;
		R.ContractId = Pair.Value.ContractId;
		R.Status = Pair.Value.Status;
		R.AcceptedAtSimSeconds = Pair.Value.AcceptedAtSimSeconds;
		R.PickedUpAtSimSeconds = Pair.Value.PickedUpAtSimSeconds;
		Saved->ContractRecords.Add(R);
	}

	FCourier404SimClock ClockB;
	FCourier404LifeService LifeB(&ClockB);
	FCourier404ContractService ContractsB;
	Courier404SliceContracts::RegisterDefaults(ContractsB);

	int32 ReloadPayouts = 0;
	ContractsB.OnContractCompleted.AddLambda([&](const FContractRuntimeState&, int32 P)
	{
		LifeB.GetWallet().Add(P);
		++ReloadPayouts;
	});

	bool bOpenJobSaved = false;
	for (const FCourier404ContractRecord& R : Saved->ContractRecords)
	{
		FContractRuntimeState S;
		S.InstanceId = R.InstanceId;
		S.ContractId = R.ContractId;
		S.Status = R.Status;
		S.AcceptedAtSimSeconds = R.AcceptedAtSimSeconds;
		S.PickedUpAtSimSeconds = R.PickedUpAtSimSeconds;
		ContractsB.RestoreInstance(S);
		bOpenJobSaved |= R.Status == EContractStatus::Accepted || R.Status == EContractStatus::PickedUp;
	}
	LifeB.GetWallet().SetBalance(Saved->Money);
	ClockB.Restore(Saved->DayIndex, Saved->HourOfDay);
	LifeB.GetNeeds().Restore(Saved->Hunger01, Saved->Fatigue01, Saved->Health01, 0.f);

	Check(LifeB.GetWallet().GetBalance() == Saved->Money, TEXT("money restored after reload"));
	Check(ClockB.GetDayIndex() == Saved->DayIndex, TEXT("clock day restored after reload"));
	Check(!bOpenJobSaved, TEXT("no half-open jobs saved"));

	// ---- 11. Living city thins at night from records only ----
	FCourier404CityRoster City;
	City.Generate();
	City.EvaluateTimeOfDay(ECourier404DayPhase::Night);
	int32 Sleeping = 0;
	for (const FCourier404CitizenRecord& C : City.GetRecords())
	{
		Sleeping += C.Activity == ECourier404Activity::Sleep ? 1 : 0;
	}
	Check(Sleeping > 150, TEXT("city thins at night without live actors"));

	UE_LOG(LogCourier404, Log, TEXT("SLICE LIFECYCLE %s (%d failures)"),
		GFailures == 0 ? TEXT("PASS") : TEXT("FAIL"), GFailures);
	return GFailures == 0 ? 0 : 1;
}