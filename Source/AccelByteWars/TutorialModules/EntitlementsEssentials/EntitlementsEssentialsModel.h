// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineError.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"

#define TEXT_NOTHING_SELECTED NSLOCTEXT("AccelByteWars", "nothing-selected", "Nothing Selected")
#define TEXT_OWNED NSLOCTEXT("AccelByteWars", "owned", "Owned")

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQueryUserEntitlementsComplete, const FOnlineError& /*Error*/, const TArray<UItemDataObject*> /*Entitlements*/)
