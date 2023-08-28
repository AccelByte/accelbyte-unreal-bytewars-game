// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineError.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Models/AccelByteUserModels.h"

#include "AccelByteWarsOnlineSessionModels.generated.h"

#pragma region "Game Session Essentials"
DECLARE_DELEGATE_TwoParams(FOnQueryUsersInfoComplete, const bool /*bSucceeded*/, const TArray<FUserOnlineAccountAccelByte*>& /*UsersInfo*/);
DECLARE_DELEGATE_TwoParams(FOnDSQueryUsersInfoComplete, const bool /*bSucceeded*/, const TArray<const FBaseUserInfo*> /*UsersInfo*/)
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnServerSessionUpdateReceived, const FName /*SessionName*/, const FOnlineError& /*Error*/, const bool /*bHasClientTravelTriggered*/)
#pragma endregion

#pragma region "Matchmaking Session Essentials"
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMatchmakingResponse, FName /*SessionName*/, bool /*bSucceeded*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingAcceptBackfillProposalComplete, bool /*bWasSuccessful*/);
#pragma endregion

#pragma region "Match Session Essentials"
/**
 * Expose the required info for Match Session Browser Menu for easier access.
 * The original data is "very nested".
 */
USTRUCT()
struct FMatchSessionEssentialInfo
{
	GENERATED_BODY()

public:
	FOnlineSessionSearchResult SessionSearchResult;

	FString OwnerName = "";
	FString OwnerAvatarUrl = "";

	EGameModeNetworkType NetworkType = EGameModeNetworkType::DS;
	EGameModeType GameModeType = EGameModeType::FFA;

	int32 RegisteredPlayerCount = 0;
	int32 MaxPlayerCount = 0;
};

/*
 * Used for Match Session Entry Widget as a way to send data
 */
UCLASS()
class ACCELBYTEWARS_API UMatchSessionData final : public UObject
{
	GENERATED_BODY()

public:
	FMatchSessionEssentialInfo SessionEssentialInfo;
	TDelegate<void(const FOnlineSessionSearchResult&)> OnJoinButtonClicked;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMatchSessionFindSessionsComplete, const TArray<FMatchSessionEssentialInfo> /*SessionEssentialsInfo*/, bool /*bSucceeded*/);

#define GAME_SESSION_REQUEST_TYPE FName(TEXT("GAMESESSIONREQUEST"))
#define GAME_SESSION_REQUEST_TYPE_MATCHSESSION FString("MATCHSESSION")
#pragma endregion 
