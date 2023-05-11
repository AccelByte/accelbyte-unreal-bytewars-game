// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/AssetManager/AccelByteWarsAssetManager.h"
#include "AccelByteWarsDataAsset.generated.h"

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Human Readable Display Name for this Asset
	UPROPERTY(EditAnywhere, Category = "Asset Info", AssetRegistrySearchable)
	FText DisplayName;

	// Description for this Asset
	UPROPERTY(EditAnywhere, Category = "Asset Info", AssetRegistrySearchable)
	FText Description;

public:
	static FText GetDisplayNameForAsset(const FPrimaryAssetId& AssetId);
	static FText GetDescriptionForAsset(const FPrimaryAssetId& AssetId);

public:
	template <typename ValueType>
	static ValueType GetMetadataForAsset(const FPrimaryAssetId& AssetId, const FName& Tag)
	{
		FAssetData AssetData;
		UAccelByteWarsAssetManager::Get().GetPrimaryAssetData(AssetId, AssetData);

		ValueType ToReturn;
		AssetData.GetTagValue(Tag, ToReturn);

		return ToReturn;
	}
	
protected:
	/** Type of this asset */
	UPROPERTY()
	FPrimaryAssetType AssetType;
};
