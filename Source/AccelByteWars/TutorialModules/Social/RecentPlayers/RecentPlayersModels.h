// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

// @@@SNIPSTART RecentPlayersModels.h-delegatemacro
DECLARE_DELEGATE_TwoParams(FOnGetRecentPlayersComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*RecentPlayersData*/)
DECLARE_DELEGATE_TwoParams(FOnGetGameSessionPlayerListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*GameSessionPlayersData*/)
// @@@SNIPEND