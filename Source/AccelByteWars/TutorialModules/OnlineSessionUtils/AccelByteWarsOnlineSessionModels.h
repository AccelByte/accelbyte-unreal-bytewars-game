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

#define BYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

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

#pragma region "Party Essentials"
#define PARTY_POPUP_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Party", "Party")
#define JOIN_NEW_PARTY_CONFIRMATION_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Join New Party Confirmation", "Leave Current Party and Join a New One?")

#define SUCCESS_SEND_PARTY_INVITE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Send Party Invite Success", "Party Invitation Sent")
#define FAILED_SEND_PARTY_INVITE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Send Party Invite Failed", "Failed to Send Party Invitation")

#define PARTY_NEW_LEADER_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "New Party Member", "{0} Is Now the Party Leader")
#define PARTY_MEMBER_JOINED_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Party Member Joined", "{0} Joined the Party")
#define PARTY_MEMBER_LEFT_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Party Member Left", "{0} Left the Party")
#define PARTY_INVITE_RECEIVED_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Party Invitation Received", "{0} Invites You to Party")
#define PARTY_INVITE_REJECTED_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Party Invitation Rejected", "{0} Rejected Your Party Invite")
#define KICKED_FROM_PARTY_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Kicked From Party", "You Are Kicked From the Party")
#define KICKED_FROM_PARTY_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Kicked From Party", "You Are Kicked From the Party")

#define ACCEPT_PARTY_INVITE_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Accept Party Invite", "Accept")
#define REJECT_PARTY_INVITE_MESSAGE NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Reject Party Invite", "Reject")
#pragma endregion
