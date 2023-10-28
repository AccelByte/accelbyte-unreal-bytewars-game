// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineError.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCheckoutComplete, const FOnlineError& /*Error*/)
DECLARE_DELEGATE_OneParam(FOnPurchaseClicked, const FString& /*CurrencyCode*/)
