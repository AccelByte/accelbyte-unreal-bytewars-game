// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "PlayWithPartyLog.h"
#include "PlayWithPartyModels.h"
#include "PlayWithPartySubsystem.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPlayWithPartySubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

// @@@SNIPSTART PlayWithPartySubsystem.h-protected
// @@@MULTISNIP PartyMatchmakingDeclaration {"selectedLines": ["1-5"]}
// @@@MULTISNIP InvitePartyGameSessionDeclaration {"selectedLines": ["1", "7-8"]}
// @@@MULTISNIP PartyGameSessionDeclaration {"selectedLines": ["1", "15-22"]}
// @@@MULTISNIP PartyGameSessionFailureDeclaration {"selectedLines": ["1", "24-31"]}
// @@@MULTISNIP PartyGameSessionValidation {"selectedLines": ["1", "33-35"]}
protected:
	void OnStartPartyMatchmakingComplete();
	void OnPartyMatchmakingComplete(FName SessionName, bool bSucceeded);
	void OnPartyMatchmakingCanceled();
	void OnPartyMatchmakingExpired(TSharedPtr<FOnlineSessionSearchAccelByte> SearchHandler);

	void InvitePartyMembersToJoinPartyGameSession(const FUniqueNetIdPtr LeaderUserId);
	void OnPartyGameSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& Invite);
	
	/**
	* @brief Update party member game session ID to the party session settings.
	* @param MemberUserId Which party members need to update their game session ID.
	* @param bResetGameSessionId If false, update the game session ID with new ID (if any). If true, remove it.
	*/
	void UpdatePartyMemberGameSession(const FUniqueNetIdPtr MemberUserId, const bool bResetGameSessionId = false);
	
	bool IsGameSessionDifferFromParty(const FUniqueNetIdPtr MemberUserId);

	void OnCreatePartyGameSessionComplete(FName SessionName, bool bSucceeded);
	void OnJoinPartyGameSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnLeavePartyGameSessionComplete(FName SessionName, bool bSucceeded);
	void OnPartyGameSessionUpdateReceived(FName SessionName);
	
	void OnPartyGameSessionFailure(const FUniqueNetId& UserId, ESessionFailure::Type FailureType);
	void OnPartyGameSessionUpdateConflictError(FName SessionName, FOnlineSessionSettings FailedSessionSettings);
	void OnPartyGameSessionServerUpdate(FName SessionName);
	void OnPartyGameSessionServerError(FName SessionName, const FString& ErrorMessage);

	void OnPartyGameSessionParticipantRemoved(FName SessionName, const FUniqueNetId& UserId);

	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& Message);

	bool ValidateToStartPartyGameSession();
	bool ValidateToJoinPartyGameSession(const FOnlineSessionSearchResult& SessionSearchResult);
	bool ValidateToStartPartyMatchmaking(const EGameModeType GameModeType);

	UAccelByteWarsOnlineSessionBase* GetOnlineSession() const;
	FOnlineSessionV2AccelBytePtr GetSessionInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubystem();
// @@@SNIPEND
};
