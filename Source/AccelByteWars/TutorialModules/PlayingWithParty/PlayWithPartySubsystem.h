// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
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
	virtual void OnStartPartyMatchmakingComplete();
	virtual void OnPartyMatchmakingComplete(FName SessionName, bool bSucceeded);
	virtual void OnPartyMatchmakingCanceled();
	virtual void OnPartyMatchmakingExpired();

	virtual void OnCreatePartyMatchComplete(FName SessionName, bool bSucceeded);
	virtual void OnJoinPartyMatchComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnPartyMatchInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& Invite);

	UAccelByteWarsOnlineSessionBase* GetOnlineSession();
	FOnlineSessionV2AccelBytePtr GetABSessionInt();
	IOnlineIdentityPtr GetIdentityInt() const;
	IOnlineUserPtr GetUserInt() const;

	UPromptSubsystem* GetPromptSubystem();
};
