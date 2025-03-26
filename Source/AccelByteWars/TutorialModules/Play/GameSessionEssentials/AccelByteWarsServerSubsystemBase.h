// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleOnlineSessionSubsystem.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "AccelByteWarsServerSubsystemBase.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsServerSubsystemBase : public UTutorialModuleOnlineSessionSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void RegisterServer(const FName SessionName){}
	virtual void UnregisterServer(const FName SessionName){}

protected:
	virtual void OnRegisterServerComplete(bool bSucceeded){}
	virtual void OnUnregisterServerComplete(bool bSucceeded){}
	
	virtual void OnServerSessionReceived(FName SessionName);

	IOnlineSessionPtr GetSessionInt() const;
	FOnlineSessionV2AccelBytePtr GetABSessionInt() const;

	void ExecuteNextTick(const FTimerDelegate & Delegate) const;

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* GameSessionOnlineSession;

	void OnLeaveSessionComplete(FName SessionName, bool bSucceeded);
	void UpdateUserCache();

	void CloseGameSession(const FOnUpdateSessionCompleteDelegate& OnComplete = {});

#pragma region "Authenticating player"
public:
	void AuthenticatePlayer(APlayerController* PlayerController);

protected:
	void AuthenticatePlayer_AddPlayerControllerToQueryQueue(APlayerController* PlayerController);
	void AuthenticatePlayer_OnRefreshSessionComplete(bool bSucceeded);
	void AuthenticatePlayer_OnQueryUserInfoComplete(const FOnlineError& Error, const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo);
	void AuthenticatePlayer_OnDSQueryUserInfoComplete(const bool bSucceeded, const TArray<const FUserDataResponse*> UserInfos);
	void AuthenticatePlayer_CompleteTask(const bool bSucceeded);

	void OnAuthenticatePlayerComplete(APlayerController* PlayerController, bool bPlayerIsInSession);
	virtual void OnAuthenticatePlayerComplete_PrePlayerSetup(APlayerController* PlayerController){}

private:
	const int32 QueryUserInfoFromSessionAttemptLimit = 1;
	int32 QueryUserInfoFromSessionAttemptLeft = QueryUserInfoFromSessionAttemptLimit;
	bool bQueryUserInfoFromSessionRunning = false;
	bool bHasReceivedSession = false;

	UPROPERTY()
	TArray<APlayerController*> QueryUserInfoFromSessionQueue;

	TMap<FUniqueNetIdRepl, TPair<FUserOnlineAccountAccelByte, int32 /*TeamIndex*/>> CachedUsersInfo;
	TMap<FString, TPair<FUserDataResponse, int32 /*TeamIndex*/>> DSCachedUsersInfo;
#pragma endregion 
};
