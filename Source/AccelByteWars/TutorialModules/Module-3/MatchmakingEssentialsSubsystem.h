// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "MatchmakingEssentialsModels.h"
#include "MatchmakingEssentialsLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "MatchmakingEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
#pragma region Module.3a Function Declarations
public:
	void StartMatchmaking(APlayerController* PC, const FString& MatchPool, const FOnMatchmakingStateChangedDelegate& OnMatchmaking);
	void CancelMatchmaking(APlayerController* PC);

protected:
	void OnDestroyToRematchmakingComplete(FName SessionName, bool bWasSuccessful, APlayerController* PC, const FString LastMatchPool);
	void OnStartMatchmakingComplete(FName SessionName, const FOnlineError& ErrorDetails, const FSessionMatchmakingResults& Results);
	void OnMatchmakingComplete(FName SessionName, bool bWasSuccessful, APlayerController* PC);
	void OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful);
#pragma endregion


#pragma region Module.3b Function Declarations
protected:
	void RegisterServer(FName SessionName);
	void GetTeamIdFromSession(FName SessionName, const FUniqueNetIdRepl& UniqueNetId, int32& OutTeamId);
	void UnregisterServer(FName SessionName);
	void OnBackfillProposalReceived(FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal);
#pragma endregion


#pragma region Module.3c Function Declarations
protected:
	void JoinSession(const FOnlineSessionSearchResult& Session, APlayerController* PC);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, APlayerController* PC);
	virtual bool TravelClient(FName SessionName, APlayerController* PC);
	void OnSessionServerUpdate(FName SessionName, APlayerController* PC);
	void OnSessionServerError(FName SessionName, const FString& ErrorMessage);
	void LeaveSession(APlayerController* PC);
#pragma endregion


public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	void BindDelegates();
	void UnbindDelegates();

	void OnServerReceivedSession(FName SessionName);
	void OnQuitGameButtonsClicked(APlayerController* PC);

	bool IsGameSessionValid(FName SessionName);
	FUniqueNetIdPtr GetPlayerUniqueNetId(APlayerController* PC) const;
	void SetTeamMemberAccelByteInformation(APlayerController* PC, TDelegate<void(bool /*bIsSuccessful*/)> OnComplete);

	TSharedPtr<FOnlineSessionSearch> CurrentMatchmakingSearchHandle;
	FDelegateHandle MatchmakingCompleteDelegateHandle;
	FDelegateHandle CancelMatchmakingCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle SessionServerUpdateDelegateHandle;
	FDelegateHandle SessionServerErrorDelegateHandle;
	FDelegateHandle BackfillProposalReceivedDelegateHandle;
	FDelegateHandle OnServerReceivedSessionDelegateHandle;

	FOnMatchmakingStateChangedDelegate OnMatchmakingHandle;
	FOnlineSessionV2AccelBytePtr SessionInterface;
};