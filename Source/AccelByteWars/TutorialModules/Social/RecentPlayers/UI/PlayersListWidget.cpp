// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PlayersListWidget.h"

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/RecentPlayers/RecentPlayersLog.h"

#include "Components/TileView.h"

void UPlayersListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RecentPlayersSubsystem = GameInstance->GetSubsystem<URecentPlayersSubsystem>();
	ensure(RecentPlayersSubsystem);
}

// @@@SNIPSTART PlayersListWidget.cpp-NativeOnActivated
void UPlayersListWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Reset widgets.
	Ws_PlayersList->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	Tv_PlayersList->ClearListItems();

	InitGameSessionPlayersList();

	SetInputModeToUIOnly();
}
// @@@SNIPEND

// @@@SNIPSTART PlayersListWidget.cpp-NativeOnDeactivated
void UPlayersListWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().Clear();
}
// @@@SNIPEND

UWidget* UPlayersListWidget::NativeGetDesiredFocusTarget() const
{
	if (Tv_PlayersList->GetListItems().IsEmpty()) 
    {
    	return Btn_Back;
    }
    return Tv_PlayersList;
}

// @@@SNIPSTART PlayersListWidget.cpp-InitGameSessionPlayersList
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "27"]}
void UPlayersListWidget::InitGameSessionPlayersList()
{
	Tv_PlayersList->OnItemClicked().RemoveAll(this);
	Tv_PlayersList->OnItemClicked().AddUObject(this, &ThisClass::OnPlayersListEntryClicked);
	
	RecentPlayersSubsystem->GetGameSessionPlayerList(GetOwningPlayer(), FOnGetGameSessionPlayerListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> PlayersData)
	{
		if(bWasSuccessful)
		{
			Tv_PlayersList->SetListItems(PlayersData);
			Ws_PlayersList->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
		}
		else
		{
			Ws_PlayersList->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		}
	}));

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if(SessionInterface.IsValid())
	{
		SessionInterface->ClearOnSessionParticipantsChangeDelegates(this);
		SessionInterface->AddOnSessionParticipantsChangeDelegate_Handle(FOnSessionParticipantsChangeDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantsChanged));
	}
}
// @@@SNIPEND

// @@@SNIPSTART PlayersListWidget.cpp-OnPlayersListEntryClicked
void UPlayersListWidget::OnPlayersListEntryClicked(UObject* Item)
{
	if (!Item)
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Unable to handle recent player entry on-click event. The recent player entry's object item is not valid."));
		return;
	}

	UFriendData* FriendData = Cast<UFriendData>(Item);
	if (!FriendData)
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Unable to handle recent player entry on-click event. The recent player entry's friend data is not valid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Unable to handle recent player entry on-click event. Base UI widget is not valid."));
		return;
	}

	UFriendDetailsWidget* DetailsWidget = Cast<UFriendDetailsWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::InGameMenu, PlayersListDetailsWidgetClass));
	if (!DetailsWidget)
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Unable to handle recent player entry on-click event. Recent player details widget is not valid."));
		return;
	}

	DetailsWidget->InitData(FriendData);
}
// @@@SNIPEND

// @@@SNIPSTART PlayersListWidget.cpp-OnSessionParticipantsChanged
void UPlayersListWidget::OnSessionParticipantsChanged(FName SessionName, const FUniqueNetId& User, bool bJoined)
{
	if(SessionName.IsEqual(NAME_GameSession))
	{
		if(!Tv_PlayersList->GetListItems().ContainsByPredicate([&User](UObject* Object)
			{
				UFriendData* PlayerData = StaticCast<UFriendData*>(Object);
				if(PlayerData)
				{
					return User == *PlayerData->UserId.Get() ;
				}
				return false;
			}))
		{
			Tv_PlayersList->ClearListItems();
			InitGameSessionPlayersList();
		}
	}
	else
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Player list not game session"));
	}
}
// @@@SNIPEND
