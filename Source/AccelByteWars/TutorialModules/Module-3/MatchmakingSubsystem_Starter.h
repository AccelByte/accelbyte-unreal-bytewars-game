// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "MatchmakingEssentialsModels.h"
#include "MatchmakingEssentialsLog.h"
#include "MatchmakingSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingSubsystem_Starter : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region Module.3a Function Declarations

public:
	// TODO: Add your public Module.3a function declarations here.

private:
	// TODO: Add your private Module.3a function declarations here.

#pragma endregion


#pragma region Module.3b Function Declarations

private:
	// TODO: Add your private Module.3b function declarations here.

#pragma endregion


#pragma region Module.3c Function Declarations

private:
	// TODO: Add your private Module.3c function declarations here.

#pragma endregion


public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

private:
	void BindDelegates();
	void UnbindDelegates();

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

	FOnMatchmakingDelegate OnMatchmakingHandle;
	FOnlineSessionAccelBytePtr SessionInterface;
};