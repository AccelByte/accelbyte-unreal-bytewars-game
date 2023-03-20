// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/Utilities/AccelByteWarsUtility.h"

#include "Core/UI/Components/MultiplayerEntries/TeamEntryWidget.h"
#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"

#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void UMatchLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	GameState = Cast<AAccelByteWarsGameStateBase>(GetWorld()->GetGameState());
	ensure(GameState);

	// Setup Countdown
	CountdownWidget->SetupWidget(
		FText::FromString("Starting Countdown"), 
		FText::FromString("Starting Match"), 
		FText::FromString("Match Starts In: "));
	CountdownWidget->CheckCountdownStateDelegate.BindUObject(this, &UMatchLobbyWidget::SetCountdownState);
	CountdownWidget->UpdateCountdownValueDelegate.BindUObject(this, &UMatchLobbyWidget::UpdateCountdownValue);
	CountdownWidget->OnCountdownFinishedDelegate.AddUObject(this, &UMatchLobbyWidget::OnCountdownFinished);
}

void UMatchLobbyWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetMatchLobbyState(EMatchLobbyState::Default);

	Btn_Start->OnClicked().AddUObject(this, &UMatchLobbyWidget::StartMatch);
	Btn_Quit->OnClicked().AddUObject(this, &UMatchLobbyWidget::LeaveMatch);
}

void UMatchLobbyWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Start->OnClicked().Clear();
	Btn_Quit->OnClicked().Clear();

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() 
	{
		ResetTeamEntries();
	}, 0.1f, false);
}

void UMatchLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime);
	GenerateMultiplayerTeamEntries();
}

void UMatchLobbyWidget::StartMatch() 
{
	if (AAccelByteWarsPlayerController* PC = Cast<AAccelByteWarsPlayerController>(GetOwningPlayer())) 
	{
		PC->TriggerLobbyStart();
	}
}

void UMatchLobbyWidget::LeaveMatch()
{
	if (OnQuitLobbyDelegate.IsBound()) 
	{
		OnQuitLobbyDelegate.Broadcast(GetOwningPlayer());
	}
	
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}

void UMatchLobbyWidget::SetMatchLobbyState(const EMatchLobbyState NewState)
{
	Ws_MatchLobby->SetActiveWidgetIndex((int)NewState);

	switch (NewState)
	{
		case EMatchLobbyState::Default:
			Btn_Start->SetUserFocus(GetOwningPlayer());
			break;
	}
}

void UMatchLobbyWidget::ResetTeamEntries()
{
	PlayerEntries.Empty();
	Panel_TeamList->ClearChildren();
}

void UMatchLobbyWidget::GenerateMultiplayerTeamEntries()
{
	// Generate team entries only on game clients.
	ENetMode NetMode = GetOwningPlayer()->GetNetMode();
	if (NetMode != ENetMode::NM_Client && NetMode != ENetMode::NM_ListenServer) 
	{
		return;
	}

	// Don't generate new player entries if it already generated.
	if (PlayerEntries.Num() == GameState->GetRegisteredPlayersNum())
	{
		return;
	}

	ResetTeamEntries();

	// Spawn team and player entry widgets.
	for (FGameplayTeamData Team : GameState->Teams) 
	{
		const TWeakObjectPtr<UTeamEntryWidget> TeamEntry = MakeWeakObjectPtr<UTeamEntryWidget>(CreateWidget<UTeamEntryWidget>(this, TeamEntryWidget.Get()));
		Panel_TeamList->AddChild(TeamEntry.Get());

		const FLinearColor TeamColor = GameInstance->GetTeamColor(Team.TeamId);
		TeamEntry->SetTeamEntryColor(TeamColor);

		// Spawn team entry widget.
		for (FGameplayPlayerData Member : Team.TeamMembers)
		{
			const AAccelByteWarsPlayerState* PlayerState = StaticCast<AAccelByteWarsPlayerState*>(UGameplayStatics::GetPlayerStateFromUniqueNetId(GetWorld(), Member.UniqueNetId));
			if (!PlayerState) 
			{
				continue;
			}

			// Spawn player entry and set the default username.
			const TWeakObjectPtr<UPlayerEntryWidget> PlayerEntry = MakeWeakObjectPtr<UPlayerEntryWidget>(CreateWidget<UPlayerEntryWidget>(this, PlayerEntryWidget.Get()));
			PlayerEntry->SetUsername(FText::FromString(PlayerState->GetPlayerName()));
			TeamEntry->AddPlayerEntry(PlayerEntry.Get());
			PlayerEntry->SetAvatarTint(TeamColor);
			PlayerEntry->SetTextColor(TeamColor);

			const FString AvatarUrl = PlayerState->AvatarURL;
			const FString AvatarId = FBase64::Encode(AvatarUrl);

			// Try to set avatar image from cache.
			FCacheBrush CacheAvatarBrush = AccelByteWarsUtility::GetImageFromCache(AvatarId);
			if (CacheAvatarBrush.IsValid())
			{
				PlayerEntry->SetAvatarTint(FLinearColor::White);
				PlayerEntry->SetAvatar(*CacheAvatarBrush.Get());
			}
			// Set avatar image from URL if it is not exists in cache.
			else if (!AvatarUrl.IsEmpty()) 
			{
				AccelByteWarsUtility::GetImageFromURL(
					AvatarUrl,
					AvatarId,
					FOnImageReceived::CreateLambda([PlayerEntry](const FCacheBrush ImageResult)
					{
						PlayerEntry->SetAvatarTint(FLinearColor::White);
						PlayerEntry->SetAvatar(*ImageResult.Get());
					})
				);
			}

			PlayerEntries.Add(PlayerEntry.Get());
		}
	}
}

ECountdownState UMatchLobbyWidget::SetCountdownState()
{
	ECountdownState State = ECountdownState::PRE;
	switch (GameState->LobbyStatus)
	{
		case ELobbyStatus::IDLE:
			State = ECountdownState::PRE;
			break;
		case ELobbyStatus::LOBBY_COUNTDOWN_STARTED:
			State = ECountdownState::COUNTING;
			break;
		case ELobbyStatus::GAME_STARTED:
			State = ECountdownState::POST;
			break;
	}

	return State;
}

int UMatchLobbyWidget::UpdateCountdownValue()
{
	return UKismetMathLibrary::FFloor(GameState->LobbyCountdown);
}

void UMatchLobbyWidget::OnCountdownFinished()
{
	SetMatchLobbyState(EMatchLobbyState::GameStarted);
}