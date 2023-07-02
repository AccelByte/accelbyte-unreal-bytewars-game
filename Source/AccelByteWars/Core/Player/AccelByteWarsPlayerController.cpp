// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerController.h"

#include "AccelByteWarsPlayerState.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

void AAccelByteWarsPlayerController::TriggerLobbyStart_Implementation()
{
	if (AAccelByteWarsMainMenuGameState* GameState = Cast<AAccelByteWarsMainMenuGameState>(GetWorld()->GetGameState()))
	{
		GameState->LobbyStatus = ELobbyStatus::GAME_STARTED;
	}
}

void AAccelByteWarsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState))
	{
		LoadingPlayerAssignment();

		AbPlayerState->OnPendingTeamAssignmentChangedDelegates.AddWeakLambda(this, [this]()
		{
			LoadingPlayerAssignment();
		});
	}
}

void AAccelByteWarsPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState))
	{
		AbPlayerState->OnPendingTeamAssignmentChangedDelegates.RemoveAll(this);
	}
}

void AAccelByteWarsPlayerController::LoadingPlayerAssignment() const
{
	if (const AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState))
	{
		if (HasLocalNetOwner())
		{
			UPromptSubsystem* PromptSubsystem = GetGameInstance()->GetSubsystem<UPromptSubsystem>();

			if (AbPlayerState->bPendingTeamAssignment)
			{
				PromptSubsystem->ShowLoading(NSLOCTEXT("AccelByteWars", "authenticating_player", "Authenticating Player"));
			}
			else
			{
				PromptSubsystem->HideLoading();
			}
		}
	}
}
