// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineError.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Core/Player/AccelByteWarsPlayerController.h"

enum ShipDesign;
enum PowerUpSelection;

#define TEXT_NOTHING_SELECTED NSLOCTEXT("AccelByteWars", "nothing-selected", "Nothing Selected")
#define TEXT_OWNED NSLOCTEXT("AccelByteWars", "owned", "Owned")

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQueryUserEntitlementsComplete, const FOnlineError& /*Error*/, const TArray<UItemDataObject*> /*Entitlements*/)
DECLARE_DELEGATE_TwoParams(FOnConsumeUserEntitlementComplete, const bool /*bSucceded*/, const UItemDataObject* /*Entitlement*/)

inline static const ShipDesign ConvertItemIdToShipDesign(const FString ItemId)
{
	if (ItemId.Equals("4f6a077395214cabade85e53f47b7a7c")) // Default Triangle Ship
	{
		return ShipDesign::TRIANGLE;
	}
	else if (ItemId.Equals("49bb99d9b20e48759bf6784bc640a936")) // D Ship
	{
		return ShipDesign::D;
	}
	else if (ItemId.Equals("112c59e29b1f4d34b782142b4e53a540")) // Double Triangle
	{
		return ShipDesign::DOUBLE_TRIANGLE;
	}
	else if (ItemId.Equals("ad8bd8a02a604796b3c9c5665e36bc1b")) // Glow Xtra
	{
		return ShipDesign::GLOW_XTRA;
	}
	else if (ItemId.Equals("a758d7fb7147465cbd4fb83b2b53d29d")) // White Star
	{
		return ShipDesign::WHITE_STAR;
	}

	return ShipDesign::TRIANGLE;
}

inline static const FString ConvertShipDesignToItemId(const ShipDesign PowerUp)
{
	FString ItemId;

	switch (PowerUp)
	{
	case ShipDesign::TRIANGLE:
		ItemId = FString("4f6a077395214cabade85e53f47b7a7c");
		break;
	case ShipDesign::D:
		ItemId = FString("49bb99d9b20e48759bf6784bc640a936");
		break;
	case ShipDesign::DOUBLE_TRIANGLE:
		ItemId = FString("112c59e29b1f4d34b782142b4e53a540");
		break;
	case ShipDesign::GLOW_XTRA:
		ItemId = FString("ad8bd8a02a604796b3c9c5665e36bc1b");
		break;
	case ShipDesign::WHITE_STAR:
		ItemId = FString("a758d7fb7147465cbd4fb83b2b53d29d");
		break;
	default:
		break;
	}

	return ItemId;
}

inline static const PowerUpSelection ConvertItemIdToPowerUp(const FString ItemId)
{
	if (ItemId.Equals("16cd5626cf4e4ea7921d4910b458c708")) // Byte Bomb
	{
		return PowerUpSelection::BYTE_BOMB;
	}
	else if (ItemId.Equals("9c9b947707484cf88b1b056afe76d3b3")) // Byte Shield
	{
		return PowerUpSelection::BYTE_SHIELD;
	}
	else if (ItemId.Equals("b147b55acf82455e9675b75d1db226a0")) // Worm Hole
	{
		return PowerUpSelection::WORM_HOLE;
	}
	else if (ItemId.Equals("afe1fc8943804f33bc9b78322c09b7e6")) // Split Missile
	{
		return PowerUpSelection::SPLIT_MISSILE;
	}

	return PowerUpSelection::NONE;
}

inline static const FString ConvertPowerUpToItemId(const PowerUpSelection PowerUp)
{
	FString ItemId;

	switch(PowerUp) 
	{
	case PowerUpSelection::BYTE_BOMB:
		ItemId = FString("16cd5626cf4e4ea7921d4910b458c708");
		break;
	case PowerUpSelection::BYTE_SHIELD:
		ItemId = FString("9c9b947707484cf88b1b056afe76d3b3");
		break;
	case PowerUpSelection::WORM_HOLE:
		ItemId = FString("b147b55acf82455e9675b75d1db226a0");
		break;
	case PowerUpSelection::SPLIT_MISSILE:
		ItemId = FString("afe1fc8943804f33bc9b78322c09b7e6");
		break;
	default:
		break;
	}

	return ItemId;
}