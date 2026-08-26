#include "City/CitySimulation.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

void FCourier404CityRoster::Generate(int32 Count)
{
	Records.Reset();
	const TCHAR* Zones[] = { TEXT("Zone.North"), TEXT("Zone.South"), TEXT("Zone.East"), TEXT("Zone.West") };

	for (int32 i = 0; i < Count; ++i)
	{
		FCourier404CitizenRecord R;
		R.Id = FName(*FString::Printf(TEXT("Citizen.%04d"), i));
		R.HomeZone = Zones[i % 4];
		R.CurrentZone = R.HomeZone;
		R.Activity = ECourier404Activity::Home;
		R.LastSimDay = 1;
		Records.Add(R);
	}
}

void FCourier404CityRoster::EvaluateTimeOfDay(ECourier404DayPhase Phase)
{
	for (int32 i = 0; i < Records.Num(); ++i)
	{
		FCourier404CitizenRecord& R = Records[i];
		if (R.bFledRecently)
		{
			R.Activity = ECourier404Activity::React; // disturbance memory persists for the phase
			continue;
		}

		switch (Phase)
		{
		case ECourier404DayPhase::Morning:
			R.Activity = (i % 3 == 0) ? ECourier404Activity::Commute : ECourier404Activity::Walk;
			break;
		case ECourier404DayPhase::Day:
			if (i % 4 == 0) { R.Activity = ECourier404Activity::Shop; }
			else if (i % 4 == 1) { R.Activity = ECourier404Activity::Sit; }
			else if (i % 4 == 2) { R.Activity = ECourier404Activity::Smoke; }
			else { R.Activity = ECourier404Activity::Walk; }
			break;
		case ECourier404DayPhase::Evening:
			R.Activity = (i % 2 == 0) ? ECourier404Activity::Shop : ECourier404Activity::Sit;
			break;
		case ECourier404DayPhase::Night:
			R.Activity = (i % 5 == 0) ? ECourier404Activity::Smoke : ECourier404Activity::Sleep;
			break;
		}

		// Shoppers/sitters drift toward the commercial south zone.
		if (R.Activity == ECourier404Activity::Shop || R.Activity == ECourier404Activity::Sit)
		{
			R.CurrentZone = TEXT("Zone.South");
		}
		else
		{
			R.CurrentZone = R.HomeZone;
		}
	}
}

int32 FCourier404CityRoster::TriggerDisturbance(const FName& ZoneId)
{
	int32 Reacted = 0;
	for (FCourier404CitizenRecord& R : Records)
	{
		if (R.CurrentZone == ZoneId && !R.bFledRecently)
		{
			R.bFledRecently = true;
			R.Activity = ECourier404Activity::React;
			++Reacted;
		}
	}
	return Reacted;
}

bool FCourier404CityRoster::IsAmbientEventActive(ECourier404DayPhase Phase, float HourOfDay)
{
	return Phase == ECourier404DayPhase::Day && HourOfDay >= 13.f && HourOfDay < 15.f; // courier unloading
}

int32 FCourier404CityRoster::GetAmbientBudget(ECourier404DayPhase Phase, int32 ScalabilityLevel)
{
	// Base pool by preset: Low stays visibly sparse but alive.
	static constexpr int32 BaseByLevel[4] = { 10, 22, 40, 48 };
	const int32 Base = BaseByLevel[FMath::Clamp(ScalabilityLevel, 0, 3)];

	float PhaseScale = 1.f;
	switch (Phase)
	{
	case ECourier404DayPhase::Morning: PhaseScale = 0.6f; break;
	case ECourier404DayPhase::Day: PhaseScale = 1.f; break;
	case ECourier404DayPhase::Evening: PhaseScale = 1.15f; break;
	case ECourier404DayPhase::Night: PhaseScale = 0.35f; break;
	}

	return FMath::Clamp(FMath::RoundToInt(Base * PhaseScale), 0, 60);
}

void FCourier404AgentPool::SyncToBudget(int32 Budget, UWorld* World)
{
	if (!World)
	{
		return;
	}

	while (Agents.Num() > Budget)
	{
		if (AActor* Agent = Agents.Last().Get())
		{
			Agent->Destroy();
		}
		Agents.Pop();
	}

	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));

	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;

	int32 Index = Agents.Num();
	while (Agents.Num() < Budget)
	{
		// Deterministic spread along the main street.
		const float X = -900.f + 150.f * ((Agents.Num() * 7) % 13);
		const float Y = 320.f - 80.f * ((Agents.Num() * 5) % 7);
		AStaticMeshActor* Agent = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), FVector(X, Y, 50.f), FRotator::ZeroRotator, Params);
		if (!Agent)
		{
			break;
		}
		if (UStaticMeshComponent* Mesh = Agent->GetStaticMeshComponent())
		{
			Mesh->SetStaticMesh(Cube);
			Mesh->SetWorldScale3D(FVector(0.35f, 0.35f, 0.9f));
			Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			Mesh->SetCanEverAffectNavigation(false);
		}
		Agents.Add(Agent);
		++Index;
	}
}