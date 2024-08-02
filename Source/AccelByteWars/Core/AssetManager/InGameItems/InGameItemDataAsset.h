// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "InGameItemUtility.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "InGameItemDataAsset.generated.h"

class UMediaSource;

UCLASS()
class ACCELBYTEWARS_API UInGameItemDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UInGameItemDataAsset();

	static const FPrimaryAssetType InGameItemAssetType;

	static FPrimaryAssetId GenerateAssetIdFromId(const FString& Id)
	{
		return FPrimaryAssetId(UInGameItemDataAsset::InGameItemAssetType, FName(*Id));
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return UInGameItemDataAsset::GenerateAssetIdFromId(Id);
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize)
	TMap<EItemSkuPlatform, FString> SkuMap = {
		{EItemSkuPlatform::AccelByte, TEXT("")},
		{EItemSkuPlatform::Steam, TEXT("")},
		{EItemSkuPlatform::Playstation, TEXT("")}
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType Type = EItemType::Skin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlateBrush DefaultImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlateBrush PreviewImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMediaSource* PreviewVideo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, MustImplement = "/Script/AccelByteWars.InGameItemInterface"))
	TSubclassOf<AActor> Actor;
};
