// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "GameModeTypeDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UGameModeTypeDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UGameModeTypeDataAsset()
	{
		AssetType = UGameModeTypeDataAsset::GameModeTypeAssetType;
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(UGameModeTypeDataAsset::GameModeTypeAssetType, GetFName());
	}
public:
	static FGameModeTypeData GetGameModeTypeDataForType(const FPrimaryAssetId& InType);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType Type = EGameModeType::FFA;
	
public:
	static const FPrimaryAssetType GameModeTypeAssetType;
};
