// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "EntitlementsEssentialsModel.generated.h"

// @@@SNIPSTART EntitlementsEssentialsModel.h-stringmacro
#define USER_EQUIPMENT_KEY FString(TEXT("UserEqipment"))
#define TEXT_SAVING_EQUIPMENTS NSLOCTEXT("AccelByteWars", "Saving Equipments", "Saving Equipments")
#define TEXT_OWNED NSLOCTEXT("AccelByteWars", "Owned", "Owned")
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsModel.h-delegatemacro
DECLARE_DELEGATE_TwoParams(FOnGetOrQueryUserEntitlementsComplete, const FOnlineError& /*Error*/, const TArray<UStoreItemDataObject*> /*Entitlements*/)
DECLARE_DELEGATE_TwoParams(FOnGetOrQueryUserItemEntitlementComplete, const FOnlineError& /*Error*/, const UStoreItemDataObject* /*Entitlement*/)
DECLARE_DELEGATE_TwoParams(FOnConsumeUserEntitlementComplete, const FOnlineError& /*Error*/, const UStoreItemDataObject* /*Entitlement*/)
DECLARE_DELEGATE_TwoParams(FOnUpdateUserEquipmentsComplete, const FOnlineError& /*Error*/, const FPlayerEquipments& /*Equipments*/)

/**
 * Delegate for tracking when a player purchased an in-game item.
 * @param BuyingPlayerNetId Unique Net ID of the player doing the purchase.
 * @param TransactionId Unity ID of the transaction.
 * @param ItemName In-game item name of the purchased item.
 * @param Amount The quantity of the item purchased.
 * @param EndAmount Total quantity of the item that the player owned.
 */
DECLARE_MULTICAST_DELEGATE_FiveParams(
	FOnItemPurchased,
	const FString& /*BuyingPlayerNetId*/,
	const FString& /*TransactionId*/,
	const FString& /*ItemName*/,
	const int /*Amount*/,
	const int /*EndAmount*/)
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsModel.h-FUserItemEntitlementRequest
struct FUserItemEntitlementRequest
{
	const FUniqueNetIdRef UserId;
	FOnGetOrQueryUserItemEntitlementComplete OnComplete;
};
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsModel.h-FConsumeEntitlementRequest
struct FConsumeEntitlementRequest
{
	const FUniqueNetIdRef UserId;
	FOnConsumeUserEntitlementComplete OnComplete;
};
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsModel.h-FPlayerEquipments
USTRUCT(BlueprintType)
struct FPlayerEquipments
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SkinId;

	UPROPERTY(BlueprintReadWrite)
	FString ColorId;

	UPROPERTY(BlueprintReadWrite)
	FString ExplosionFxId;

	UPROPERTY(BlueprintReadWrite)
	FString MissileTrailFxId;

	UPROPERTY(BlueprintReadWrite)
	FString PowerUpId;
};
// @@@SNIPEND
