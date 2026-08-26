#include "Tools/ContentAuditCommandlet.h"
#include "Courier404.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/SoftObjectPath.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"

UContentAuditCommandlet::UContentAuditCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

namespace
{
	int32 GViolations = 0;

	bool IsPowerOfTwo(int32 Value)
	{
		return Value > 0 && ((Value & (Value - 1)) == 0);
	}

	void CheckTexture2D(UTexture2D* Texture, const FString& AssetPath)
	{
		if (!Texture)
		{
			return;
		}

		const int32 Width = Texture->GetSizeX();
		const int32 Height = Texture->GetSizeY();
		const int32 MaxDim = FMath::Max(Width, Height);

		if (!IsPowerOfTwo(MaxDim))
		{
			UE_LOG(LogCourier404, Error, TEXT("AUDIT VIOLATION: Texture %s dimensions (%dx%d) not power of two"),
				*AssetPath, Width, Height);
			++GViolations;
			return;
		}

		if (MaxDim > 4096)
		{
			UE_LOG(LogCourier404, Error, TEXT("AUDIT VIOLATION: Texture %s exceeds 4096 limit (%dx%d)"),
				*AssetPath, Width, Height);
			++GViolations;
			return;
		}

		if (MaxDim == 4096)
		{
			UE_LOG(LogCourier404, Warning, TEXT("AUDIT NOTE: Texture %s at 4096 requires justification (%dx%d)"),
				*AssetPath, Width, Height);
		}
	}

	void CheckStaticMesh(UStaticMesh* Mesh, const FString& AssetPath)
	{
		if (!Mesh)
		{
			return;
		}

		const int32 NumLODs = Mesh->GetNumLODs();

		if (NumLODs < 1)
		{
			UE_LOG(LogCourier404, Error, TEXT("AUDIT VIOLATION: Mesh %s has no LODs"), *AssetPath);
			++GViolations;
			return;
		}

		if (!Mesh->IsNaniteEnabled() && NumLODs < 2)
		{
			UE_LOG(LogCourier404, Warning, TEXT("AUDIT NOTE: Non-Nanite mesh %s has only %d LOD(s), expected >= 2"),
				*AssetPath, NumLODs);
		}
	}

	void CheckMaterial(UMaterial* Material, const FString& AssetPath)
	{
		if (!Material)
		{
			return;
		}

		if (Material->GetMaterialDomain() == EMaterialDomain::MD_UI)
		{
			return;
		}

		const int32 NumExpressions = Material->GetExpressions().Num();
		if (NumExpressions > 50)
		{
			UE_LOG(LogCourier404, Warning, TEXT("AUDIT NOTE: Material %s has %d expressions, consider using instances"),
				*AssetPath, NumExpressions);
		}
	}

	void CheckUnusedAsset(IAssetRegistry& AssetRegistry, const FAssetData& AssetData, const FString& AssetPath)
	{
		TArray<FAssetData> Referencers;
		AssetRegistry.GetReferencers(FName(AssetData.PackageName), Referencers);

		if (Referencers.Num() == 0)
		{
			UE_LOG(LogCourier404, Warning, TEXT("AUDIT NOTE: Asset %s has no referencers, may be unused"),
				*AssetPath);
		}
	}
}

int32 UContentAuditCommandlet::Main(const FString& Params)
{
	GViolations = 0;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataArray;
	AssetRegistry.GetAssetsByPath(FName("/Game"), AssetDataArray, true);

	int32 TextureCount = 0;
	int32 MeshCount = 0;
	int32 MaterialCount = 0;

	for (const FAssetData& AssetData : AssetDataArray)
	{
		const FString AssetPath = AssetData.GetObjectPathString();
		const FName AssetClass = AssetData.AssetClassPath.GetName();

		if (AssetClass == TEXT("Texture2D"))
		{
			++TextureCount;
			if (UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset()))
			{
				CheckTexture2D(Texture, AssetPath);
			}
		}
		else if (AssetClass == TEXT("StaticMesh"))
		{
			++MeshCount;
			if (UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset()))
			{
				CheckStaticMesh(Mesh, AssetPath);
			}
		}
		else if (AssetClass == TEXT("Material"))
		{
			++MaterialCount;
			if (UMaterial* Material = Cast<UMaterial>(AssetData.GetAsset()))
			{
				CheckMaterial(Material, AssetPath);
			}
		}

		CheckUnusedAsset(AssetRegistry, AssetData, AssetPath);
	}

	AssetDataArray.Empty();
	AssetRegistry.GetAssetsByClass(UMaterialInstance::StaticClass()->GetClassPathName(), AssetDataArray);

	for (const FAssetData& AssetData : AssetDataArray)
	{
		const FString AssetPath = AssetData.GetObjectPathString();
		UMaterialInstance* Instance = Cast<UMaterialInstance>(AssetData.GetAsset());
		if (!Instance)
		{
			continue;
		}

		UMaterialInterface* Parent = Instance->GetBaseMaterial();
		if (!Parent)
		{
			UE_LOG(LogCourier404, Error, TEXT("AUDIT VIOLATION: MaterialInstance %s has no parent master material"), *AssetPath);
			++GViolations;
		}
	}

	UE_LOG(LogCourier404, Log, TEXT("AUDIT SUMMARY: %d textures, %d meshes, %d materials scanned"),
		TextureCount, MeshCount, MaterialCount);
	UE_LOG(LogCourier404, Log, TEXT("AUDIT RESULT: %d violation(s)"), GViolations);

	if (GViolations == 0)
	{
		UE_LOG(LogCourier404, Log, TEXT("CONTENT AUDIT PASS"));
	}
	else
	{
		UE_LOG(LogCourier404, Error, TEXT("CONTENT AUDIT FAIL"));
	}

	return GViolations == 0 ? 0 : 1;
}
