// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "MatchmakingEssentialsLog.h"
#include "MatchmakingEssentialsSubsystem.generated.h"

enum class EMatchmakingState
{
	None,
	FindingMatch,
	JoiningMatch,
	CancelingMatch,
	FindMatchFailed,
	MatchFound
};

class UPlayerEntryWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmaking, EMatchmakingState /*MatchmakingState*/);
typedef FOnMatchmaking::FDelegate FOnMatchmakingDelegate;

UCLASS()
class ACCELBYTEWARS_API UMatchmakingEssentialsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	void StartMatchmaking(APlayerController* PC, const FString& MatchPool, const FOnMatchmakingDelegate& OnMatchmaking);
	void CancelMatchmaking(APlayerController* PC);

private:
	void RegisterServer(FName SessionName);
	void UnregisterServer(FName SessionName);

	void JoinSession(const FOnlineSessionSearchResult& Session);
	void LeaveSession(APlayerController* PC);

	void OnStartMatchmakingComplete(FName SessionName, const FOnlineError& ErrorDetails, const FSessionMatchmakingResults& Results);
	void OnMatchmakingComplete(FName SessionName, bool bWasSuccessful);
	void OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionServerUpdate(FName SessionName);
	void OnDestroyToRematchmakingComplete(FName SessionName, bool bWasSuccessful);

	void OnBackfillProposalReceived(FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal);

	void GenerateOnlineTeamEntries(TArray<FUniqueNetIdPtr> PlayerUniqueNetIds, TArray<UPlayerEntryWidget*> PlayerEntryWidgets);
	int32 GetTeamIdFromSession(APlayerController* PC);
	void TravelClient(FName SessionName);

	bool IsGameSessionValid(FName SessionName);
	FUniqueNetIdPtr GetAssociatedPlayerUniqueId() const;

	TSharedPtr<FOnlineSessionSearch> CurrentMatchmakingSearchHandle;
	FDelegateHandle MatchmakingCompleteDelegateHandle;
	FDelegateHandle CancelMatchmakingCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle SessionServerUpdateDelegateHandle;
	FDelegateHandle BackfillProposalReceivedDelegateHandle;
	
	APlayerController* AssociatedPC;
	FString LastMatchPool;
	FOnMatchmakingDelegate OnMatchmakingHandle;

	FOnlineSessionAccelBytePtr SessionInterface;
};