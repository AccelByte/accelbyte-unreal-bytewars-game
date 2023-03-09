// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"
#include "Core/Player/AccelByteWarsPlayerController.h"

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
	OnQuitLobbyDelegate.ExecuteIfBound(GetOwningPlayer());
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

void UMatchLobbyWidget::GenerateMultiplayerTeamEntries(const bool bIsOnline)
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
	TArray<FUniqueNetIdPtr> PlayerNetIds;
	for (FGameplayTeamData Team : GameState->Teams) 
	{
		const TWeakObjectPtr<UTeamEntryWidget> TeamEntry = MakeWeakObjectPtr<UTeamEntryWidget>(CreateWidget<UTeamEntryWidget>(this, TeamEntryWidget.Get()));
		Panel_TeamList->AddChild(TeamEntry.Get());

		const FLinearColor TeamColor = GameInstance->GetTeamColor(Team.TeamId);
		TeamEntry->SetTeamEntryColor(TeamColor);

		// Spawn team entry widget.
		for (FGameplayPlayerData Member : Team.TeamMembers)
		{
			// Save player net id. It will be used to get player's information (username, avatar URL, etc) from backend.
			const FUniqueNetIdPtr PlayerNetId = Member.UniqueNetId.GetUniqueNetId();
			if (!ensure(PlayerNetId.IsValid()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Player net id us not valid."));
				continue;
			}
			PlayerNetIds.Add(PlayerNetId);

			// Spawn player entry and set the default username.
			const TWeakObjectPtr<UPlayerEntryWidget> PlayerEntry = MakeWeakObjectPtr<UPlayerEntryWidget>(CreateWidget<UPlayerEntryWidget>(this, PlayerEntryWidget.Get()));
			const FText DefaultUsername = FText::FromString(FString::Printf(TEXT("Player %d"), PlayerEntries.Num() + 1));
			PlayerEntry->SetUsername(DefaultUsername);
			TeamEntry->AddPlayerEntry(PlayerEntry.Get());
			PlayerEntry->SetAvatarTint(TeamColor);
			PlayerEntry->SetTextColor(TeamColor);
			
			PlayerEntries.Add(PlayerEntry.Get());
		}
	}

	// Update players username and avatars. Fetch it from AccelByte backend.
	if (bIsOnline) 
	{
		OnGenerateOnlineTeamEntries.ExecuteIfBound(PlayerNetIds, PlayerEntries);
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