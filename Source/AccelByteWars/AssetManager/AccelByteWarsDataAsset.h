// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AssetManager/AccelByteWarsAssetManager.h"
#include "AccelByteWarsDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	/** Type of this asset */
	UPROPERTY()
	FPrimaryAssetType AssetType;

public:
	// Human Readable Display Name for this Asset
	UPROPERTY(EditAnywhere, Category = "UI", AssetRegistrySearchable)
	FText DisplayName;
	// Description for this Asset
	UPROPERTY(EditAnywhere, Category = "UI", AssetRegistrySearchable)
	FText Description;

	// Display Image for this Asset
	// UPROPERTY(EditAnywhere, Category = "UI|Images", AssetRegistrySearchable)
	// TSoftObjectPtr<UTexture2D> DisplayImage;

	// TODO: store reference to asset manager here

public:
	static FText GetDisplayNameForAsset(const FPrimaryAssetId& AssetId);
	static FText GetDescriptionForAsset(const FPrimaryAssetId& AssetId);

public:
	template <typename ValueType>
	static ValueType GetMetadataForAsset(const FPrimaryAssetId& AssetId, const FName& Tag) {
		FAssetData AssetData;
		UAccelByteWarsAssetManager::Get().GetPrimaryAssetData(AssetId, AssetData);

		ValueType ToReturn;
		AssetData.GetTagValue(Tag, ToReturn);

		return ToReturn;
	}
};
