// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Models/AccelByteEcommerceModels.h"

#define TEXT_BALANCE_ERROR NSLOCTEXT("AccelByteWars", "balance_error", "NaN")

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetWalletInfoComplete, bool /*bWasSuccessful*/, const FAccelByteModelsWalletInfo& /*Response*/);
