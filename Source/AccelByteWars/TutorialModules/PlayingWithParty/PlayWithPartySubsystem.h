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

	void InvitePartyMembersToJoinPartyMatch(const FUniqueNetIdPtr LeaderUserId);
	void OnPartyMatchInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& Invite);
	
	void UpdatePartyMemberGameSession(const FUniqueNetIdPtr MemberUserId);
	bool IsGameSessionDifferFromParty(const FUniqueNetIdPtr MemberUserId);

	void OnCreatePartyMatchComplete(FName SessionName, bool bSucceeded);
	void OnJoinPartyMatchComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnLeavePartyMatchComplete(FName SessionName, bool bSucceeded);

	bool ValidateToStartSession();
	bool ValidateToJoinSession(const FOnlineSessionSearchResult& SessionSearchResult);
	bool ValidateToStartMatchmaking(const EGameModeType GameModeType);

	UAccelByteWarsOnlineSessionBase* GetOnlineSession() const;
	FOnlineSessionV2AccelBytePtr GetSessionInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubystem();
};
