#include "Tools/BuildDistrictCommandlet.h"
#include "Courier404.h"
#include "Core/Courier404LocationTags.h"
#include "Interaction/Courier404DropPoint.h"
#include "Interaction/Courier404FoodSource.h"
#include "Interaction/Courier404Package.h"
#include "Interaction/Courier404Bed.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "GameFramework/PlayerStart.h"
#include "Components/BoxComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

UBuildDistrictCommandlet::UBuildDistrictCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

namespace
{
	const TCHAR* MapPackageName = TEXT("/Game/Maps/District01");

	AStaticMeshActor* SpawnBox(UWorld* World, const FVector& Location, const FVector& BoxExtent,
		const FName Tag = NAME_None, const FLinearColor* = nullptr)
	{
		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transactional;
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, Params);
		if (!Actor)
		{
			return nullptr;
		}

		UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
		if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
		{
			Mesh->SetStaticMesh(Cube);
		}
		Mesh->SetWorldScale3D(BoxExtent / 50.f); // engine cube is 100uu
		Mesh->SetCollisionProfileName(TEXT("BlockAll"));

		if (!Tag.IsNone())
		{
			Actor->Tags.Add(Tag);
		}
		return Actor;
	}

	AStaticMeshActor* SpawnLamp(UWorld* World, const FVector& Location)
	{
		AStaticMeshActor* Pole = SpawnBox(World, Location, FVector(6.f, 6.f, 160.f));
		if (!Pole)
		{
			return nullptr;
		}

		UPointLightComponent* Light = NewObject<UPointLightComponent>(Pole);
		Light->SetupAttachment(Pole->GetRootComponent());
		Light->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
		Light->SetIntensity(1200.f);
		Light->SetLightColor(FLinearColor(1.f, 0.82f, 0.6f));
		Light->SetAttenuationRadius(900.f);
		Light->SetCastShadows(false); // decorative lights stay cheap per performance budget
		Light->RegisterComponent();

		return Pole;
	}
}

int32 UBuildDistrictCommandlet::Main(const FString& Params)
{
	const FString MapPath = FPackageName::LongPackageNameToFilename(MapPackageName, FPackageName::GetMapPackageExtension());

	// Fully load any previous build so the package never sits half-streamed,
	// then wipe its actors; generation below repopulates deterministically.
	FlushAsyncLoading();
	UWorld* World = LoadObject<UWorld>(nullptr, TEXT("/Game/Maps/District01.District01"));
	if (World)
	{
		ULevel* Existing = World->PersistentLevel;
		for (int32 Idx = Existing->Actors.Num() - 1; Idx >= 0; --Idx)
		{
			if (AActor* Actor = Existing->Actors[Idx])
			{
				Actor->SetLifeSpan(0.f);      // immediate
				Actor->Destroy();             // runtime destroy works headless
			}
		}
	}
	else
	{
		UPackage* NewPackage = CreatePackage(MapPackageName);
		World = UWorld::CreateWorld(EWorldType::GamePreview, false, FName(TEXT("District01")), NewPackage);
	}
	if (!World)
	{
		UE_LOG(LogCourier404, Error, TEXT("Failed to create world"));
		return 1;
	}

	ULevel* Level = World->PersistentLevel;

	auto SpawnInLevel = [World](TSubclassOf<AActor> Class, const FVector& Loc, const FRotator& Rot) -> AActor*
	{
		FActorSpawnParameters P;
		P.ObjectFlags |= RF_Transactional;
		P.OverrideLevel = World->PersistentLevel;
		return World->SpawnActor(Class, &Loc, &Rot, P);
	};

	// -- Ground plane --
	SpawnBox(World, FVector(0, 0, -50), FVector(4000, 3000, 50));

	// -- Streets: two crossing roads (visual strips slightly raised, walkable) --
	SpawnBox(World, FVector(0, 0, 2), FVector(1200, 200, 3));  // main street E-W
	SpawnBox(World, FVector(0, 0, 2), FVector(200, 1500, 3)); // avenue N-S

	// -- Building masses around streets (occluders) --
	struct FBuilding { float X, Y; float EX, EY, EZ; };
	const FBuilding Buildings[] = {
		{-800,  500, 250, 200, 250}, {-800, -500, 250, 200, 220},
		{ 800,  500, 250, 200, 260}, { 800, -500, 250, 200, 240},
		{-350,  900, 180, 160, 200}, { 350,  900, 180, 160, 210},
		{-350, -900, 180, 160, 230}, { 350, -900, 180, 160, 205},
	};
	for (const FBuilding& B : Buildings)
	{
		SpawnBox(World, FVector(B.X, B.Y, B.EZ), FVector(B.EX, B.EY, B.EZ));
	}

	// -- Apartment interior (small room shell + practicals) at west end --
	SpawnBox(World, FVector(-1100, 100, 130), FVector(150, 140, 10), FName(Courier404Locations::Apartment)); // floor
	SpawnBox(World, FVector(-1235, 100, 140), FVector(15, 140, 130)); // wall
	SpawnBox(World, FVector(-965, 100, 140), FVector(15, 140, 130));  // wall
	{
		// Warm practical inside apartment.
		AStaticMeshActor* Anchor = SpawnBox(World, FVector(-1100, 100, 268), FVector(20, 20, 6));
		UPointLightComponent* Lamp = NewObject<UPointLightComponent>(Anchor);
		Lamp->SetupAttachment(Anchor->GetRootComponent());
		Lamp->SetRelativeLocation(FVector(0, 0, -12));
		Lamp->SetIntensity(600.f);
		Lamp->SetLightColor(FLinearColor(1.f, 0.75f, 0.5f));
		Lamp->SetAttenuationRadius(500.f);
		Lamp->SetCastShadows(true); // interior key light
		Lamp->RegisterComponent();
	}

	// -- Parking area next to apartment --
	SpawnBox(World, FVector(-900, -250, 4), FVector(160, 120, 4), FName(Courier404Locations::Parking));

	// -- Convenience store (south of main street) with usable food counter --
	SpawnBox(World, FVector(-200, -450, 90), FVector(120, 80, 80), FName(Courier404Locations::Store));
	if (ACourier404FoodSource* Food = Cast<ACourier404FoodSource>(
		SpawnInLevel(ACourier404FoodSource::StaticClass(), FVector(-200, -360, 50), FRotator(0.f, 180.f, 0.f))))
	{
		Food->MealPrice = 15;
	}

	// -- Bed inside the apartment --
	SpawnInLevel(ACourier404Bed::StaticClass(), FVector(-1150, 100, 160), FRotator(0.f, 90.f, 0.f));

	// -- Ordinary pickup point --
	SpawnBox(World, FVector(500, -300, 30), FVector(60, 60, 20), FName(Courier404Locations::PickupOrdinary));

	// -- Physical parcel waiting at the ordinary pickup point --
	if (!SpawnInLevel(ACourier404Package::StaticClass(), FVector(500, -300, 60), FRotator::ZeroRotator))
	{
		UE_LOG(LogCourier404, Error, TEXT("Failed to spawn pickup package"));
		return 1;
	}

	// -- Ordinary drop-off point (gameplay binding) --
	if (ACourier404DropPoint* Drop = Cast<ACourier404DropPoint>(
		SpawnInLevel(ACourier404DropPoint::StaticClass(), FVector(700, 300, 40), FRotator::ZeroRotator)))
	{
		Drop->DropPointId = TEXT("Point.DropA");
		Drop->Tags.Add(FName(Courier404Locations::DropOrdinary));
	}
	else
	{
		UE_LOG(LogCourier404, Error, TEXT("Failed to spawn ordinary drop point"));
		return 1;
	}

	// -- Anonymous locker + its sealed bag + risky destination --
	SpawnBox(World, FVector(-500, 300, 40), FVector(50, 40, 60), FName(Courier404Locations::LockerAnonymous));
	if (ACourier404Package* SealedBag = Cast<ACourier404Package>(
		SpawnInLevel(ACourier404Package::StaticClass(), FVector(-500, 250, 60), FRotator::ZeroRotator)))
	{
		SealedBag->CargoId = TEXT("Cargo.SealedBag");
	}

	if (ACourier404DropPoint* Risky = Cast<ACourier404DropPoint>(
		SpawnInLevel(ACourier404DropPoint::StaticClass(), FVector(1000, -700, 60), FRotator::ZeroRotator)))
	{
		Risky->DropPointId = TEXT("Point.RiskyDrop");
		Risky->Tags.Add(FName(Courier404Locations::DropRisky));
	}

	// -- Police route marker (patrol path anchor) --
	SpawnBox(World, FVector(0, 250, 8), FVector(30, 30, 8), FName(Courier404Locations::PoliceRoute));

	// -- Hostile courtyard --
	SpawnBox(World, FVector(650, -650, 8), FVector(120, 100, 8), FName(Courier404Locations::HostileAlley));

	// -- Night street lighting along main street --
	for (const float X : {-900.f, -300.f, 300.f, 900.f})
	{
		SpawnLamp(World, FVector(X, 260, 0));
		SpawnLamp(World, FVector(X, -260, 0));
	}

	// -- Global rig: sun/sky/fog/exposure --
	if (ADirectionalLight* Sun = Cast<ADirectionalLight>(
		SpawnInLevel(ADirectionalLight::StaticClass(), FVector(0, 0, 500), FRotator(-35.f, 25.f, 0.f))))
	{
		Sun->GetLightComponent()->SetIntensity(2.5f);
		Sun->GetLightComponent()->SetLightColor(FLinearColor(1.f, 0.93f, 0.85f));
		Sun->GetLightComponent()->SetCastShadows(true);
	}

	if (ASkyLight* Sky = Cast<ASkyLight>(
		SpawnInLevel(ASkyLight::StaticClass(), FVector(0, 0, 600), FRotator::ZeroRotator)))
	{
		Sky->GetLightComponent()->SetIntensity(1.f);
		Sky->GetLightComponent()->SetMobility(EComponentMobility::Stationary);
	}

	if (AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(
		SpawnInLevel(AExponentialHeightFog::StaticClass(), FVector(0, 0, 0), FRotator::ZeroRotator)))
	{
		Fog->GetComponent()->SetFogDensity(0.02f);
		Fog->GetComponent()->SetStartDistance(1500.f);
	}

	{
		APostProcessVolume* PPV = World->SpawnActor<APostProcessVolume>();
		PPV->bUnbound = true;
		FPostProcessSettings Settings;
		Settings.bOverride_AutoExposureMinBrightness = true;
		Settings.bOverride_AutoExposureMaxBrightness = true;
		Settings.AutoExposureMinBrightness = 0.7f;
		Settings.AutoExposureMaxBrightness = 1.2f;
		Settings.bOverride_VignetteIntensity = true;
		Settings.VignetteIntensity = 0.3f;
		PPV->Settings = Settings;
	}

	// -- Player start inside the apartment --
	if (!SpawnInLevel(APlayerStart::StaticClass(), FVector(-1100, 100, 170), FRotator(0.f, 0.f, 0.f)))
	{
		UE_LOG(LogCourier404, Error, TEXT("Failed to spawn PlayerStart"));
		return 1;
	}

	World->UpdateWorldComponents(false, false);

	World->SetFlags(RF_Public | RF_Standalone);
	if (ULevel* Level0 = World->PersistentLevel)
	{
		Level0->SetFlags(RF_Public | RF_Standalone);
	}

	// -- Save as map package --
	UPackage* MapPackage = World->GetOutermost();
	World->SetFlags(RF_Public | RF_Standalone);
	if (ULevel* Level0 = World->PersistentLevel)
	{
		Level0->SetFlags(RF_Public | RF_Standalone);
	}
	FSavePackageArgs Args;
	Args.TopLevelFlags = RF_Public | RF_Standalone;
	if (!UPackage::SavePackage(MapPackage, World, *MapPath, Args))
	{
		UE_LOG(LogCourier404, Error, TEXT("Failed to save map %s"), *MapPath);
		return 1;
	}

	FAssetRegistryModule::AssetCreated(World);

	int32 LandmarkCount = 0;
	const TCHAR* LandmarkTags[] = {
		Courier404Locations::Apartment, Courier404Locations::Parking, Courier404Locations::Store,
		Courier404Locations::PickupOrdinary, Courier404Locations::DropOrdinary,
		Courier404Locations::LockerAnonymous, Courier404Locations::DropRisky,
		Courier404Locations::PoliceRoute, Courier404Locations::HostileAlley
	};
	for (FActorIterator It(World); It; ++It)
	{
		for (const TCHAR* Tag : LandmarkTags)
		{
			if (It->ActorHasTag(FName(Tag)))
			{
				++LandmarkCount;
				break;
			}
		}
	}
	UE_LOG(LogCourier404, Log, TEXT("District built: %d landmark-tagged actors, saved to %s"),
		LandmarkCount, *MapPath);
	return 0;
}

