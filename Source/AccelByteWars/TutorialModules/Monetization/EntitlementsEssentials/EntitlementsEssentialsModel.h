// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineError.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"

#define TEXT_NOTHING_SELECTED NSLOCTEXT("AccelByteWars", "nothing-selected", "Nothing Selected")
#define TEXT_OWNED NSLOCTEXT("AccelByteWars", "owned", "Owned")

DECLARE_DELEGATE_TwoParams(FOnGetOrQueryUserEntitlementsComplete, const FOnlineError& /*Error*/, const TArray<UStoreItemDataObject*> /*Entitlements*/)
DECLARE_DELEGATE_TwoParams(FOnGetOrQueryUserItemEntitlementComplete, const FOnlineError& /*Error*/, const UStoreItemDataObject* /*Entitlement*/)
DECLARE_DELEGATE_TwoParams(FOnConsumeUserEntitlementComplete, const FOnlineError& /*Error*/, const UStoreItemDataObject* /*Entitlement*/)

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
