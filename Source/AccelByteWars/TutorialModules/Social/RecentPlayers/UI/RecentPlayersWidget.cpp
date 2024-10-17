// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RecentPlayersWidget.h"

#include "Social/RecentPlayers/RecentPlayersLog.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Components/TileView.h"
#include "CommonButtonBase.h"

void URecentPlayersWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RecentPlayersSubsystem = GameInstance->GetSubsystem<URecentPlayersSubsystem>();
	ensure(RecentPlayersSubsystem);
}

// @@@SNIPSTART RecentPlayersWidget.cpp-NativeOnActivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-11", "16"]}
// @@@MULTISNIP PutItAllTogether {"highlightedLines": "{13-15}"}
void URecentPlayersWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Reset widgets.
	Ws_RecentPlayers->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	Tv_RecentPlayers->ClearListItems();

	Tv_RecentPlayers->OnItemClicked().AddUObject(this, &ThisClass::OnRecentPlayerEntryClicked);
	
	OnQueryRecentPlayersCompletedDelegate = FOnQueryRecentPlayersCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryRecentPlayerComplete);
	RecentPlayersSubsystem->BindRecentPlayerDelegate(OnQueryRecentPlayersCompletedDelegate);
	InitRecentPlayersList();
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-4", "7-8"]}
// @@@MULTISNIP PutItAllTogether {"highlightedLines": "{5}"}
void URecentPlayersWidget::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();
	Tv_RecentPlayers->OnItemClicked().Clear();
	RecentPlayersSubsystem->UnBindRecentPlayerDelegate(OnRecentPlayersListUpdatedDelegateHandle);

	Super::NativeOnDeactivated();
}
// @@@SNIPEND

UWidget* URecentPlayersWidget::NativeGetDesiredFocusTarget() const
{
	if (Tv_RecentPlayers->GetListItems().IsEmpty()) 
	{
		return Btn_Back;
	}
	return Tv_RecentPlayers;
}

void URecentPlayersWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

// @@@SNIPSTART RecentPlayersWidget.cpp-InitRecentPlayersList
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "27"]}
void URecentPlayersWidget::InitRecentPlayersList()
{
	ensure(RecentPlayersSubsystem);
	RecentPlayersSubsystem->GetRecentPlayers(GetOwningPlayer(), FOnGetRecentPlayersComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> RecentPlayersList)
	{
		if(bWasSuccessful)
		{
			if(RecentPlayersList.IsEmpty())
			{
				Ws_RecentPlayers->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
				Tv_RecentPlayers->ClearListItems();

				RecentPlayersSubsystem->QueryRecentPlayers(GetOwningPlayer());
			}
			else
			{
				Tv_RecentPlayers->SetListItems(RecentPlayersList);
				Ws_RecentPlayers->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
			}
		}
		else
		{
			Ws_RecentPlayers->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
			Tv_RecentPlayers->ClearListItems();
		}
	}));
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersWidget.cpp-OnRecentPlayerEntryClicked
void URecentPlayersWidget::OnRecentPlayerEntryClicked(UObject* Item) 
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

	UFriendDetailsWidget* DetailsWidget = Cast<UFriendDetailsWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, RecentPlayerDetailsWidgetClass));
	if (!DetailsWidget)
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Unable to handle recent player entry on-click event. Recent player details widget is not valid."));
		return;
	}

	DetailsWidget->InitData(FriendData);
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersWidget.cpp-OnQueryRecentPlayerComplete
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-3", "25"]}
void URecentPlayersWidget::OnQueryRecentPlayerComplete(const FUniqueNetId& UserId, const FString& Namespace,
	bool bWasSuccessful, const FString& Error)
{
	if(!bWasSuccessful)
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Error querying recent players! Message: %s"), *Error);
		Ws_RecentPlayers->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	RecentPlayersSubsystem->GetRecentPlayers(GetOwningPlayer(), FOnGetRecentPlayersComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> RecentPlayersList)
	{
		if(bWasSuccessful)
		{
			Tv_RecentPlayers->SetListItems(RecentPlayersList);
			Ws_RecentPlayers->SetWidgetState(RecentPlayersList.IsEmpty() ?
				EAccelByteWarsWidgetSwitcherState::Empty :
				EAccelByteWarsWidgetSwitcherState::Not_Empty);
		}
		else
		{
			Ws_RecentPlayers->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);			
		}
	}));
}
// @@@SNIPEND
