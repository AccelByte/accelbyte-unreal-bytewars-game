// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-3/MatchmakingSubsystem_Starter.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

#include "Core/AssetManager/AssetManagementSubsystem.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

#include "Core/GameModes/AccelByteWarsGameModeBase.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"


void UMatchmakingSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get Online Subsystem and make sure it's valid.
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
		return;
	}

	// Grab the reference of AccelByte Session Interface and make sure it's valid.
	SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Session Interface is not valid."));
		return;
	}

	// Bind delegates to game events if the matchmaking module is active.
	if (UAssetManagementSubsystem* AssetManagement = GetGameInstance()->GetSubsystem<UAssetManagementSubsystem>())
	{
		UAccelByteWarsDataAsset* DataAsset = AssetManagement->GetByteWarsAssetManager()->GetAssetFromCache(FPrimaryAssetId{ "TutorialModule:MATCHMAKINGESSENTIALS" });
		if (!DataAsset)
		{
			return;
		}

		UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(DataAsset);
		if (!TutorialModule || !TutorialModule->bIsActive)
		{
			return;
		}

		AAccelByteWarsGameModeBase::OnAddOnlineMemberDelegate.AddUObject(this, &ThisClass::SetTeamMemberAccelByteInformation);
		UMatchLobbyWidget::OnQuitLobbyDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);
		UPauseWidget::OnQuitGameDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);
		UGameOverWidget::OnQuitGameDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);

		BindDelegates();
	}
}

void UMatchmakingSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	AAccelByteWarsGameModeBase::OnAddOnlineMemberDelegate.Clear();
	UMatchLobbyWidget::OnQuitLobbyDelegate.Clear();
	UPauseWidget::OnQuitGameDelegate.Clear();
	UGameOverWidget::OnQuitGameDelegate.Clear();

	UnbindDelegates();
}

bool UMatchmakingSubsystem_Starter::IsGameSessionValid(FName SessionName)
{
	return (SessionName == NAME_GameSession);
}

FUniqueNetIdPtr UMatchmakingSubsystem_Starter::GetPlayerUniqueNetId(APlayerController* PC) const
{
	if (!ensure(PC))
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

void UMatchmakingSubsystem_Starter::SetTeamMemberAccelByteInformation(APlayerController* PC, TDelegate<void(bool /*bIsSuccessful*/)> OnComplete)
{
	const FUniqueNetIdPtr PlayerNetId = PC->PlayerState->GetUniqueId().GetUniqueNetId();
	if (!PlayerNetId.IsValid())
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Player Net Id is invalid. Cannot get player AccelByte's information."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	const FUniqueNetIdAccelByteUserPtr AccelBytePlayerNetId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerNetId);
	if (!ensure(AccelBytePlayerNetId.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Player's AccelByte Net Id us not valid. Cannot get player AccelByte's information."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PC->PlayerState);
	if (!PlayerState)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Player State is null. Cannot get player's AccelByte information."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	FRegistry::User.BulkGetUserInfo(
		{ AccelBytePlayerNetId->GetAccelByteId() },
		THandler<FListBulkUserInfo>::CreateWeakLambda(this, [PlayerState, OnComplete](const FListBulkUserInfo& Result)
		{
			const FBaseUserInfo PlayerInfo = Result.Data[0];
			if (!PlayerInfo.Username.IsEmpty())
			{
				PlayerState->SetPlayerName(PlayerInfo.Username);
			}

			PlayerState->AvatarURL = PlayerInfo.AvatarUrl;

			OnComplete.ExecuteIfBound(true);
		}),
		FErrorHandler::CreateWeakLambda(this, [OnComplete](int32 ErrorCode, const FString& ErrorMessage)
		{
			OnComplete.ExecuteIfBound(false);
		})
	);
}

#pragma region Module.3 General Function Definitions
void UMatchmakingSubsystem_Starter::BindDelegates()
{
	// TODO: Bind your necessary delegates here.
}

void UMatchmakingSubsystem_Starter::UnbindDelegates()
{
	// TODO: Unbind your delegates here.
}

void UMatchmakingSubsystem_Starter::OnQuitGameButtonsClicked(APlayerController* PC)
{
	// TODO: Call leave game session here.
}
#pragma endregion


#pragma region Module.3a Function Definitions

// TODO: Add your Module.3a function definitions here.

#pragma endregion 


#pragma region Module.3b Function Definitions

// TODO: Add your Module.3b function definitions here.

#pragma endregion


#pragma region Module.3c Function Definitions

// TODO: Add your Module.3c function definitions here.

#pragma endregion