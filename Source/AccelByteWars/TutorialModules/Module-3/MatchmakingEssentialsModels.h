// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

enum class EMatchmakingState
{
	Default,
	FindingMatch,
	JoiningMatch,
	CancelingMatch,
	FindMatchFailed,
	MatchFound
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStateChanged, EMatchmakingState /*MatchmakingState*/);
typedef FOnMatchmakingStateChanged::FDelegate FOnMatchmakingStateChangedDelegate;