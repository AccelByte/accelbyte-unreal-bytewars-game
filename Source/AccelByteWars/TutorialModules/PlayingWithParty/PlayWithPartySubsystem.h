// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "TutorialModules/PlayingWithParty/PlayWithPartyLog.h"
#include "TutorialModules/PlayingWithParty/PlayWithPartyModels.h"
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

protected:
	void OnStartPartyMatchmakingComplete();
	void OnPartyMatchmakingComplete(FName SessionName, bool bSucceeded);
	void OnPartyMatchmakingCanceled();
	void OnPartyMatchmakingExpired();

	void InvitePartyMembersToJoinPartyGameSession(const FUniqueNetIdPtr LeaderUserId);
	void OnPartyGameSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& Invite);
	
	void UpdatePartyMemberGameSession(const FUniqueNetIdPtr MemberUserId);
	bool IsGameSessionDifferFromParty(const FUniqueNetIdPtr MemberUserId);

	void OnCreatePartyGameSessionComplete(FName SessionName, bool bSucceeded);
	void OnJoinPartyGameSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnLeavePartyGameSessionComplete(FName SessionName, bool bSucceeded);

	bool ValidateToStartPartyGameSession();
	bool ValidateToJoinPartyGameSession(const FOnlineSessionSearchResult& SessionSearchResult);
	bool ValidateToStartPartyMatchmaking(const EGameModeType GameModeType);

	UAccelByteWarsOnlineSessionBase* GetOnlineSession() const;
	FOnlineSessionV2AccelBytePtr GetSessionInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubystem();
};
