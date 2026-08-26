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

	SaveAssetPackage(Move->GetOutermost());
	SaveAssetPackage(Look->GetOutermost());
	SaveAssetPackage(Interact->GetOutermost());

	UPackage* ImcPackage = EnsureAssetPackage(TEXT("IMC_CourierDefault"));
	UInputMappingContext* IMC = FindObject<UInputMappingContext>(ImcPackage, TEXT("IMC_CourierDefault"));
	if (!IMC)
	{
		IMC = NewObject<UInputMappingContext>(ImcPackage, TEXT("IMC_CourierDefault"), RF_Public | RF_Standalone);
	}
	IMC->UnmapAll();

	auto MapKey = [IMC](UInputAction* Action, const FKey Key, bool bNegateX, bool bNegateY)
	{
		FEnhancedActionKeyMapping& Mapping = IMC->MapKey(Action, Key);
		if (bNegateX || bNegateY)
		{
			UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(IMC);
			Negate->bX = bNegateX;
			Negate->bY = bNegateY;
			Negate->bZ = false;
			Mapping.Modifiers.Add(Negate);
		}
	};

	MapKey(Move, EKeys::W, false, false);
	MapKey(Move, EKeys::S, false, true);
	MapKey(Move, EKeys::A, true, false);
	MapKey(Move, EKeys::D, false, false);
	MapKey(Look, EKeys::Mouse2D, false, false);
	MapKey(Interact, EKeys::E, false, false);

	FAssetRegistryModule::AssetCreated(IMC);
	SaveAssetPackage(ImcPackage);

	UE_LOG(LogCourier404, Log, TEXT("Input asset generation complete: IA_CourierMove, IA_CourierLook, IA_CourierInteract, IMC_CourierDefault"));
	return 0;
}
