// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleOnlineSession.h"
#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "SessionEssentialsOnlineSession.generated.h"

UCLASS()
class ACCELBYTEWARS_API USessionEssentialsOnlineSession : public UAccelByteWarsOnlineSessionBase
{
	GENERATED_BODY()

public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

	virtual FNamedOnlineSession* GetSession(const FName SessionName) override;
	virtual EAccelByteV2SessionType GetSessionType(const FName SessionName) override;
	virtual FName GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType) override;

	virtual void CreateSession(
		const int32 LocalUserNum,
		FName SessionName,
		FOnlineSessionSettings SessionSettings,
		const EAccelByteV2SessionType SessionType,
		const FString& SessionTemplateName) override;
	virtual void JoinSession(
		const int32 LocalUserNum,
		FName SessionName,
		const FOnlineSessionSearchResult& SearchResult) override;
	virtual void LeaveSession(FName SessionName) override;

	virtual FOnCreateSessionComplete* GetOnCreateSessionCompleteDelegates() override
	{
		return &OnCreateSessionCompleteDelegates;
	}
	virtual FOnJoinSessionComplete* GetOnJoinSessionCompleteDelegates() override
	{
		return &OnJoinSessionCompleteDelegates;
	}
	virtual FOnDestroySessionComplete* GetOnLeaveSessionCompleteDelegates() override
	{
		return &OnLeaveSessionCompleteDelegates;
	}

protected:
	bool bLeaveSessionRunning = false;

	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded) override;
	/*The parent's function with the same name will not be used. Ignore the "hides a non-virtual function" warning*/
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) override;

private:
	const FName GameSessionName = NAME_GameSession;
	const FName PartySessionName = NAME_PartySession;

	FOnCreateSessionComplete OnCreateSessionCompleteDelegates;
	FOnJoinSessionComplete OnJoinSessionCompleteDelegates;
	FOnDestroySessionComplete OnLeaveSessionCompleteDelegates;

	void OnLeaveSessionForCreateSessionComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const FOnlineSessionSettings SessionSettings);
	FDelegateHandle OnLeaveSessionForCreateSessionCompleteDelegateHandle;

	void OnLeaveSessionForJoinSessionComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const FOnlineSessionSearchResult SearchResult);
	FDelegateHandle OnLeaveSessionForJoinSessionCompleteDelegateHandle;
};
