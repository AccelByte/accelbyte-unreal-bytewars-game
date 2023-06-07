// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "MatchmakingEssentialsModels.h"
#include "MatchmakingEssentialsLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "MatchmakingSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

#pragma region Module.3a Function Declarations

public:
	// TODO: Add your public Module.3a function declarations here.

protected:
	// TODO: Add your protected Module.3a function declarations here.

#pragma endregion


#pragma region Module.3b Function Declarations

protected:
	// TODO: Add your protected Module.3b function declarations here.

#pragma endregion


#pragma region Module.3c Function Declarations

protected:
	// TODO: Add your protected Module.3c function declarations here.

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
	FDelegateHandle BackfillProposalReceivedDelegateHandle;
	FDelegateHandle OnServerReceivedSessionDelegateHandle;

	FOnMatchmakingStateChangedDelegate OnMatchmakingHandle;
	FOnlineSessionV2AccelBytePtr SessionInterface;
};