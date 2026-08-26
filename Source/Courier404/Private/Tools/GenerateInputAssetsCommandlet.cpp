#include "Tools/GenerateInputAssetsCommandlet.h"
#include "Courier404.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

UGenerateInputAssetsCommandlet::UGenerateInputAssetsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

namespace
{
	constexpr const TCHAR* InputRoot = TEXT("/Game/Input");

	void EnsureInputDir()
	{
		IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Input")), true);
	}

	UPackage* EnsureAssetPackage(const TCHAR* AssetName)
	{
		const FString PackagePath = FString(InputRoot) / AssetName;
		if (UPackage* Existing = FindPackage(nullptr, *PackagePath))
		{
			Existing->FullyLoad();
			return Existing;
		}
		UPackage* Package = CreatePackage(*PackagePath);
		Package->FullyLoad();
		return Package;
	}

	UInputAction* EnsureAction(const TCHAR* AssetName, const EInputActionValueType ValueType)
	{
		UPackage* Package = EnsureAssetPackage(AssetName);
		UInputAction* Action = FindObject<UInputAction>(Package, *FName(AssetName).ToString());
		if (!Action)
		{
			Action = NewObject<UInputAction>(Package, FName(AssetName), RF_Public | RF_Standalone);
		}
		Action->ValueType = ValueType;
		FAssetRegistryModule::AssetCreated(Action);
		return Action;
	}

	void SaveAssetPackage(UPackage* Package)
	{
		const FString FileName = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		UPackage::SavePackage(Package, nullptr, *FileName, Args);
	}
}

int32 UGenerateInputAssetsCommandlet::Main(const FString& Params)
{
	EnsureInputDir();

	UInputAction* Move = EnsureAction(TEXT("IA_CourierMove"), EInputActionValueType::Axis2D);
	UInputAction* Look = EnsureAction(TEXT("IA_CourierLook"), EInputActionValueType::Axis2D);
	UInputAction* Interact = EnsureAction(TEXT("IA_CourierInteract"), EInputActionValueType::Boolean);
	UInputAction* Drive = EnsureAction(TEXT("IA_CourierDrive"), EInputActionValueType::Axis2D);
	UInputAction* Brake = EnsureAction(TEXT("IA_CourierBrake"), EInputActionValueType::Boolean);

	SaveAssetPackage(Move->GetOutermost());
	SaveAssetPackage(Look->GetOutermost());
	SaveAssetPackage(Interact->GetOutermost());
	SaveAssetPackage(Drive->GetOutermost());
	SaveAssetPackage(Brake->GetOutermost());

	UPackage* ImcPackage = EnsureAssetPackage(TEXT("IMC_CourierDefault"));
	UInputMappingContext* IMC = FindObject<UInputMappingContext>(ImcPackage, TEXT("IMC_CourierDefault"));
	if (!IMC)
	{
		IMC = NewObject<UInputMappingContext>(ImcPackage, TEXT("IMC_CourierDefault"), RF_Public | RF_Standalone);
	}
	IMC->UnmapAll();

	auto MapKeyTo = [](UInputMappingContext* Context, UInputAction* Action, const FKey Key, bool bNegateX, bool bNegateY)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		if (bNegateX || bNegateY)
		{
			UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(Context);
			Negate->bX = bNegateX;
			Negate->bY = bNegateY;
			Negate->bZ = false;
			Mapping.Modifiers.Add(Negate);
		}
	};

	MapKeyTo(IMC, Move, EKeys::W, false, false);
	MapKeyTo(IMC, Move, EKeys::S, false, true);
	MapKeyTo(IMC, Move, EKeys::A, true, false);
	MapKeyTo(IMC, Move, EKeys::D, false, false);
	MapKeyTo(IMC, Look, EKeys::Mouse2D, false, false);
	MapKeyTo(IMC, Interact, EKeys::E, false, false);

	FAssetRegistryModule::AssetCreated(IMC);
	SaveAssetPackage(ImcPackage);

	// Vehicle driving context.
	UPackage* DriveImcPackage = EnsureAssetPackage(TEXT("IMC_CourierDrive"));
	UInputMappingContext* DriveIMC = FindObject<UInputMappingContext>(DriveImcPackage, TEXT("IMC_CourierDrive"));
	if (!DriveIMC)
	{
		DriveIMC = NewObject<UInputMappingContext>(DriveImcPackage, TEXT("IMC_CourierDrive"), RF_Public | RF_Standalone);
	}
	DriveIMC->UnmapAll();

	MapKeyTo(DriveIMC, Drive, EKeys::W, false, false);
	MapKeyTo(DriveIMC, Drive, EKeys::S, false, true);
	MapKeyTo(DriveIMC, Drive, EKeys::A, true, false);
	MapKeyTo(DriveIMC, Drive, EKeys::D, false, false);
	MapKeyTo(DriveIMC, Brake, EKeys::SpaceBar, false, false);

	FAssetRegistryModule::AssetCreated(DriveIMC);
	SaveAssetPackage(DriveImcPackage);

	UE_LOG(LogCourier404, Log, TEXT("Input asset generation complete: IA_CourierMove, IA_CourierLook, IA_CourierInteract, IA_CourierDrive, IA_CourierBrake, IMC_CourierDefault, IMC_CourierDrive"));
	return 0;
}
