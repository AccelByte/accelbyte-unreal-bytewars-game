// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

DECLARE_DELEGATE_TwoParams(FOnRetrieveCrossplayPreferenceCompleted, bool /*bSucceeded*/, bool /*bEnabled*/)
DECLARE_DELEGATE_OneParam(FOnUpdateCrossplayPreferenceCompleted, bool /*bSucceeded*/)

#define TEXT_LOADING NSLOCTEXT("AccelByteWars", "Loading", "Loading")
#define TEXT_SAVING NSLOCTEXT("AccelByteWars", "Saving", "Saving")
#define TEXT_CROSSPLAY_CHECKBOX_NAME NSLOCTEXT("AccelByteWars", "Crossplay enabled", "Enable crossplay matchmaking")
