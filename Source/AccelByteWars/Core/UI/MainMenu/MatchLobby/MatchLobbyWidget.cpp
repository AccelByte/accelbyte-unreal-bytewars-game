// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Utilities/AccelByteWarsUtility.h"

#include "Core/UI/Components/MultiplayerEntries/TeamEntryWidget.h"
#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void UMatchLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	GameState = Cast<AAccelByteWarsMainMenuGameState>(GetWorld()->GetGameState());
	ensure(GameState);
}

void UMatchLobbyWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// show loading screen on server start travel
	GameState->OnIsServerTravellingChanged.AddUniqueDynamic(this, &ThisClass::ShowLoading);

	if (GameState->GameSetup.StartGameCountdown != INDEX_NONE)
	{
		// Setup Countdown
		CountdownWidget->SetupWidget(
			FText::FromString("Starting Countdown"),
			FText::FromString("Starting Match"),
			FText::FromString("Match Starts In: "));
		CountdownWidget->CheckCountdownStateDelegate.BindUObject(this, &UMatchLobbyWidget::SetCountdownState);
		CountdownWidget->UpdateCountdownValueDelegate.BindUObject(this, &UMatchLobbyWidget::UpdateCountdownValue);
		CountdownWidget->OnCountdownFinishedDelegate.AddUObject(this, &UMatchLobbyWidget::OnCountdownFinished);
	}
	else
	{
		CountdownWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	QueryTeamMembersInfo();

	SetMatchLobbyState(EMatchLobbyState::Default);

	Btn_Start->OnClicked().AddUObject(this, &UMatchLobbyWidget::StartMatch);
	Btn_Quit->OnClicked().AddUObject(this, &UMatchLobbyWidget::LeaveMatch);

	GameState->OnTeamsChanged.AddUObject(this, &ThisClass::QueryTeamMembersInfo);

	// if on P2P game, only enable Btn_Start for host
	if (GameState->GameSetup.NetworkType == EGameModeNetworkType::P2P)
	{
		Btn_Start->SetVisibility(GetOwningPlayer()->HasAuthority() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMatchLobbyWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Start->OnClicked().Clear();
	Btn_Quit->OnClicked().Clear();

	GameState->OnTeamsChanged.RemoveAll(this);

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
}

void UMatchLobbyWidget::ShowLoading()
{
	DeinitializeFTUEDialogues();
	SetMatchLobbyState(EMatchLobbyState::GameStarted);
}

// @@@SNIPSTART MatchLobbyWidget.cpp-StartMatch
void UMatchLobbyWidget::StartMatch() 
{
	if (AAccelByteWarsPlayerController* PC = Cast<AAccelByteWarsPlayerController>(GetOwningPlayer())) 
	{
		PC->TriggerLobbyStart();
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchLobbyWidget.cpp-LeaveMatch
void UMatchLobbyWidget::LeaveMatch()
{
	if (OnQuitLobbyDelegate.IsBound()) 
	{
		OnQuitLobbyDelegate.Broadcast(GetOwningPlayer());
	}
	
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}
// @@@SNIPEND

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
	Panel_TeamList->ClearChildren();
}

void UMatchLobbyWidget::QueryTeamMembersInfo()
{
	Ws_TeamList->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Collect team member user ids.
	TArray<FUniqueNetIdRef> MemberUserIds{};
	for (const FGameplayTeamData& Team : GameState->Teams)
	{
		for (const FGameplayPlayerData& Member : Team.TeamMembers)
		{
			if (Member.UniqueNetId.IsValid() && Member.UniqueNetId.GetUniqueNetId())
			{
				MemberUserIds.Add(Member.UniqueNetId.GetUniqueNetId().ToSharedRef());
			}
		}
	}

	// Broadcast to query user information from backend.
	if (OnQueryTeamMembersInfoDelegate.IsBound()) 
	{
		OnQueryTeamMembersInfoDelegate.Broadcast(
			GetOwningPlayer(),
			MemberUserIds,
			TDelegate<void(const TMap<FUniqueNetIdRepl, FGameplayPlayerData>&)>::CreateUObject(this, &ThisClass::GenerateMultiplayerTeamEntries));
	}
	else 
	{
		GenerateMultiplayerTeamEntries();
	}
}

// @@@SNIPSTART MatchLobbyWidget.cpp-GenerateMultiplayerTeamEntries
void UMatchLobbyWidget::GenerateMultiplayerTeamEntries(const TMap<FUniqueNetIdRepl, FGameplayPlayerData>& AdditionalMembersInfo)
{
	// Generate team entries only on game clients.
	// Also, don't attempt to generate entries when the Listen Server is tearing down.
	ENetMode NetMode = GetOwningPlayer()->GetNetMode();
	if ((NetMode != ENetMode::NM_Client && NetMode != ENetMode::NM_ListenServer) || GetWorld()->bIsTearingDown)
	{
		return;
	}

	ResetTeamEntries();

	// Spawn team and player entry widgets.
	const FUniqueNetIdRepl LocalPlayerNetId = GetGameInstance()->GetPrimaryPlayerUniqueIdRepl();
	int32 PlayerIndex = 0;
	for (const FGameplayTeamData& Team : GameState->Teams) 
	{
		if (Team.TeamMembers.IsEmpty()) continue;

		const TWeakObjectPtr<UTeamEntryWidget> TeamEntry = MakeWeakObjectPtr<UTeamEntryWidget>(CreateWidget<UTeamEntryWidget>(this, TeamEntryWidget.Get()));
		Panel_TeamList->AddChild(TeamEntry.Get());

		const FLinearColor TeamColor = GameInstance->GetTeamColor(Team.TeamId);
		TeamEntry->SetTeamEntryColor(TeamColor);

		// Spawn team entry widget.
		for (const FGameplayPlayerData& Member : Team.TeamMembers)
		{
			PlayerIndex++;

			FGameplayPlayerData* AdditionalInfo = nullptr;
			if (AdditionalMembersInfo.Contains(Member.UniqueNetId))
			{
				AdditionalInfo = new FGameplayPlayerData(AdditionalMembersInfo[Member.UniqueNetId]);
			}
			
			FString PlayerName = FString::Printf(TEXT("Player %d"), PlayerIndex);
			const bool bIsLocalPlayer = LocalPlayerNetId.IsValid() && LocalPlayerNetId == Member.UniqueNetId;
			if (bIsLocalPlayer)
			{
				PlayerName = NSLOCTEXT("AccelByteWars", "Match Lobby Widget Local Player Identifier", "You").ToString();
			}
			else if (!Member.PlayerName.IsEmpty())
			{
				PlayerName = Member.PlayerName;
			}

			// Spawn player entry and set the default username.
			const TWeakObjectPtr<UPlayerEntryWidget> PlayerEntry = MakeWeakObjectPtr<UPlayerEntryWidget>(CreateWidget<UPlayerEntryWidget>(this, PlayerEntryWidget.Get()));
			TeamEntry->AddPlayerEntry(PlayerEntry.Get());
			PlayerEntry->SetUsername(FText::FromString(PlayerName));
			PlayerEntry->SetAvatar(AdditionalInfo ? AdditionalInfo->AvatarURL : Member.AvatarURL);
			PlayerEntry->SetAvatarTint(Member.AvatarURL.IsEmpty() ? TeamColor : FLinearColor::White);
			PlayerEntry->SetTextColor(TeamColor);
			PlayerEntry->SetNetId(Member.UniqueNetId.GetUniqueNetId());
			PlayerEntry->ActivateWidget();
		}
	}

	Ws_TeamList->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
}
// @@@SNIPEND

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
}