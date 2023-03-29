// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

#define FAILED_MESSAGE_NONE FString(TEXT("No error on matchmaking process."))
#define FAILED_FIND_MATCH FString(TEXT("Failed to find a match. Please try again."))
#define FAILED_CANCEL_MATCH FString(TEXT("Failed to cancel matchmaking."))
#define FAILED_JOIN_MATCH FString(TEXT("Failed to join the match. Please try again."))
#define FAILED_FIND_SERVER FString(TEXT("Failed to travel to the game server. Game server not found. Please try again."))

enum class EMatchmakingState : uint8
{
	Default = 0,
	StartMatchmaking,
	FindingMatch,
	JoiningMatch,
	CancelingMatch,
	FindMatchFailed,
	MatchFound
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMatchmakingStateChanged, EMatchmakingState /*MatchmakingState*/, FString /* ErrorMessage */);
typedef FOnMatchmakingStateChanged::FDelegate FOnMatchmakingStateChangedDelegate;