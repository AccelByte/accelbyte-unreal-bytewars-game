// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PresenceWidget_Starter.h"

#include "CommonUILibrary.h"
#include "CommonTextBlock.h"
#include "Components/Throbber.h"
#include "Components/ListViewBase.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/FriendsEssentials/UI/FriendWidgetEntry.h"
#include "Social/FriendsEssentials/UI/FriendWidgetEntry_Starter.h"
#include "Social/NativeFriendsEssentials/UI/NativeFriendWidgetEntry.h"
#include "Social/NativeFriendsEssentials/UI/NativeFriendWidgetEntry_Starter.h"
#include "Social/ManagingFriends/UI/BlockedPlayerWidgetEntry.h"
#include "Social/ManagingFriends/UI/BlockedPlayerWidgetEntry_Starter.h"

void UPresenceWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	SetupPresence();
}

void UPresenceWidget_Starter::NativeDestruct()
{
	// TODO: Add your code here.

	Super::NativeDestruct();
}

void UPresenceWidget_Starter::SetupPresence()
{
	// Initially, hide the presence.
	Tb_Presence->SetVisibility(ESlateVisibility::Collapsed);
	Th_Loader->SetVisibility(ESlateVisibility::Visible);

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. GameInstance is not valid."));
		return;
	}

	// Get presence subsystem.
	PresenceEssentialsSubsystem = GetGameInstance()->GetSubsystem<UPresenceEssentialsSubsystem_Starter>();
	if (!PresenceEssentialsSubsystem)
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. Presence Essentials subsystem is not valid."));
		return;
	}

	/* The presence widget is a modular widget, it can be spawned to the friend entry and the friend details widget. 
	 * This is a part of requirement to separate between presence module with other tutorial modules. 
	 * But since the presence module depends on the friend essentials module, this widget needs to get the user id from friend essentials widgets. */

	// If the presence is spawned in the an entry widget, then get the user id from the it.
	if (UAccelByteWarsWidgetEntry* WidgetEntry = Cast<UAccelByteWarsWidgetEntry>(UCommonUILibrary::FindParentWidgetOfType(this, UAccelByteWarsWidgetEntry::StaticClass())))
	{
		if (UFriendWidgetEntry* FriendWidgetEntry = Cast<UFriendWidgetEntry>(WidgetEntry)) 
		{
			FriendWidgetEntry->GetOnListItemObjectSet()->RemoveAll(this);
			FriendWidgetEntry->GetOnListItemObjectSet()->AddWeakLambda(this, [this, FriendWidgetEntry]()
			{
				const UFriendData* CachedFriendData = Cast<UFriendData>(FriendWidgetEntry->GetListItem());
				if (CachedFriendData &&
					CachedFriendData->UserId &&
					CachedFriendData->UserId.IsValid())
				{
					PresenceUserId = CachedFriendData->UserId;
					ParentListView = FriendWidgetEntry->GetOwningListView();
					OnSetupPresenceComplete();
				}
				else
				{
					UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. User Id is not valid."));
				}
			});
		}
		else if (UFriendWidgetEntry_Starter* FriendWidgetEntry_Starter = Cast<UFriendWidgetEntry_Starter>(WidgetEntry))
		{
			FriendWidgetEntry_Starter->GetOnListItemObjectSet()->RemoveAll(this);
			FriendWidgetEntry_Starter->GetOnListItemObjectSet()->AddWeakLambda(this, [this, FriendWidgetEntry_Starter]()
			{
				const UFriendData* CachedFriendData = Cast<UFriendData>(FriendWidgetEntry_Starter->GetListItem());
				if (CachedFriendData &&
					CachedFriendData->UserId &&
					CachedFriendData->UserId.IsValid())
				{
					PresenceUserId = CachedFriendData->UserId;
					ParentListView = FriendWidgetEntry_Starter->GetOwningListView();
					OnSetupPresenceComplete();
				}
				else
				{
					UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. User Id is not valid."));
				}
			});
		}
		else if (UBlockedPlayerWidgetEntry* BlockedPlayerWidgetEntry = Cast<UBlockedPlayerWidgetEntry>(WidgetEntry))
		{
			BlockedPlayerWidgetEntry->GetOnListItemObjectSet()->RemoveAll(this);
			BlockedPlayerWidgetEntry->GetOnListItemObjectSet()->AddWeakLambda(this, [this, BlockedPlayerWidgetEntry]()
			{
				const UFriendData* CachedFriendData = Cast<UFriendData>(BlockedPlayerWidgetEntry->GetListItem());
				if (CachedFriendData &&
					CachedFriendData->UserId &&
					CachedFriendData->UserId.IsValid())
				{
					PresenceUserId = CachedFriendData->UserId;
					ParentListView = BlockedPlayerWidgetEntry->GetOwningListView();
					OnSetupPresenceComplete();
				}
				else
				{
					UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. User Id is not valid."));
				}
			});
		}
		else if (UBlockedPlayerWidgetEntry_Starter* BlockedPlayerWidgetEntry_Starter = Cast<UBlockedPlayerWidgetEntry_Starter>(WidgetEntry))
		{
			BlockedPlayerWidgetEntry_Starter->GetOnListItemObjectSet()->RemoveAll(this);
			BlockedPlayerWidgetEntry_Starter->GetOnListItemObjectSet()->AddWeakLambda(this, [this, BlockedPlayerWidgetEntry_Starter]()
			{
				const UFriendData* CachedFriendData = Cast<UFriendData>(BlockedPlayerWidgetEntry_Starter->GetListItem());
				if (CachedFriendData &&
					CachedFriendData->UserId &&
					CachedFriendData->UserId.IsValid())
				{
					PresenceUserId = CachedFriendData->UserId;
					ParentListView = BlockedPlayerWidgetEntry_Starter->GetOwningListView();
					OnSetupPresenceComplete();
				}
				else
				{
					UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. User Id is not valid."));
				}
			});
		}
		// Native Friend List
		else if (UNativeFriendWidgetEntry* NativeFriendWidgetEntry = Cast<UNativeFriendWidgetEntry>(WidgetEntry))
		{
			NativeFriendWidgetEntry->GetOnListItemObjectSet()->RemoveAll(this);
			NativeFriendWidgetEntry->GetOnListItemObjectSet()->AddWeakLambda(this, [this, NativeFriendWidgetEntry]()
			{
				const UNativeFriendData* CachedFriendData = Cast<UNativeFriendData>(NativeFriendWidgetEntry->GetListItem());
				if (CachedFriendData &&
					CachedFriendData->UserId &&
					CachedFriendData->UserId.IsValid())
				{
					PresenceUserId = CachedFriendData->UserId;
					ParentListView = NativeFriendWidgetEntry->GetOwningListView();
					OnSetupPresenceComplete();
				}
				else
				{
					UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. User Id is not valid."));
				}
			});
		}
		else if (UNativeFriendWidgetEntry_Starter* NativeFriendWidgetEntry_Starter = Cast<UNativeFriendWidgetEntry_Starter>(WidgetEntry))
		{
			NativeFriendWidgetEntry_Starter->GetOnListItemObjectSet()->RemoveAll(this);
			NativeFriendWidgetEntry_Starter->GetOnListItemObjectSet()->AddWeakLambda(this, [this, NativeFriendWidgetEntry_Starter]()
			{
				const UNativeFriendData* CachedFriendData = Cast<UNativeFriendData>(NativeFriendWidgetEntry_Starter->GetListItem());
				if (CachedFriendData &&
					CachedFriendData->UserId &&
					CachedFriendData->UserId.IsValid())
				{
					PresenceUserId = CachedFriendData->UserId;
					ParentListView = NativeFriendWidgetEntry_Starter->GetOwningListView();
					OnSetupPresenceComplete();
				}
				else
				{
					UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. User Id is not valid."));
				}
			});
		}
		return;
	}

	// If the presence is spawned in the friend details widget, then get the user id from the it.
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	if (!BaseUIWidget)
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. Base UI widget is not valid."));
		return;
	}

	const AAccelByteWarsInGameGameState* GameState = Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState());
	const EBaseUIStackType StackType = (GameState == nullptr) ? EBaseUIStackType::Menu : EBaseUIStackType::InGameMenu;
	
	UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(StackType, this);
	if (!ParentWidget)
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. Widget doesn't have valid parent widget."));
		return;
	}

	if (const UFriendDetailsWidget* FriendDetailsWidget = Cast<UFriendDetailsWidget>(ParentWidget))
	{
		if (FriendDetailsWidget->GetCachedFriendData() &&
			FriendDetailsWidget->GetCachedFriendData()->UserId &&
			FriendDetailsWidget->GetCachedFriendData()->UserId.IsValid())
		{
			PresenceUserId = FriendDetailsWidget->GetCachedFriendData()->UserId;
			OnSetupPresenceComplete();
		}
		else 
		{
			UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. User Id is not valid."));
		}

		return;
	}
}

void UPresenceWidget_Starter::OnSetupPresenceComplete()
{
	// TODO: Add your code here.
}

#pragma region Module Presence Essentials Function Definitions
// TODO: Add your function definitions here.
#pragma endregion
