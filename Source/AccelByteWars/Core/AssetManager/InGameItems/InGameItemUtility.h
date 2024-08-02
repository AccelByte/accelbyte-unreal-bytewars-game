// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "InGameItemUtility.generated.h"

class UInGameItemDataAsset;
UENUM(BlueprintType)
enum class EItemSkuPlatform : uint8
{
	AccelByte,
	Playstation,
	Steam,
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Skin,
	PowerUp
};

UINTERFACE(BlueprintType)
class UInGameItemInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IInGameItemInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	virtual void OnEquip(){}
	virtual void OnUse(){}
	virtual void DestroyItem(){}
	virtual EItemType GetType(){ return EItemType::Skin; }
};

UCLASS(BlueprintType)
class UInGameItemUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Item")
	static UInGameItemDataAsset* GetItemDataAsset(const FString& ItemId);

	UFUNCTION(BlueprintPure, Category = "Item")
	static UInGameItemDataAsset* GetItemDataAssetBySku(
		const EItemSkuPlatform Platform,
		const FString& Sku);
};
