// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PresenceWidget.h"

#include "CommonUILibrary.h"
#include "CommonTextBlock.h"
#include "Components/Throbber.h"
#include "Components/ListViewBase.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/FriendsEssentials/UI/FriendWidgetEntry.h"
#include "Social/FriendsEssentials/UI/FriendWidgetEntry_Starter.h"
#include "Social/ManagingFriends/UI/BlockedPlayerWidgetEntry.h"
#include "Social/ManagingFriends/UI/BlockedPlayerWidgetEntry_Starter.h"

void UPresenceWidget::NativeConstruct()
{
	Super::NativeConstruct();

    SetupPresence();
}

void UPresenceWidget::NativeDestruct()
{
    // Clear cache.
    PresenceUserId = nullptr;
    ParentListView = nullptr;

    // Unbind presence events.
    PresenceEssentialsSubsystem->GetOnPresenceReceivedDelegates()->RemoveAll(this);
    PresenceEssentialsSubsystem->GetOnBulkQueryPresenceCompleteDelegates()->RemoveAll(this);

    Super::NativeDestruct();
}

void UPresenceWidget::SetupPresence()
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
    PresenceEssentialsSubsystem = GetGameInstance()->GetSubsystem<UPresenceEssentialsSubsystem>();
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

        return;
    }

    // If the presence is spawned in the friend details widget, then get the user id from the it.
    UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
    if (!BaseUIWidget)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to setup presence widget. Base UI widget is not valid."));
        return;
    }

    UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
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

void UPresenceWidget::OnSetupPresenceComplete()
{
    // Bind presence events.
    if (!PresenceEssentialsSubsystem->GetOnBulkQueryPresenceCompleteDelegates()->IsBoundToObject(this))
    {
        PresenceEssentialsSubsystem->GetOnPresenceReceivedDelegates()->AddUObject(this, &ThisClass::OnPresenceUpdated);
        PresenceEssentialsSubsystem->GetOnBulkQueryPresenceCompleteDelegates()->AddUObject(this, &ThisClass::OnBulkQueryPresenceComplete);
    }

    // Get and display presence.
    RefreshPresence();
}

void UPresenceWidget::RefreshPresence()
{
    if (!PresenceUserId) 
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to get presence to be displayed on the widget. User Id is not valid."));
        return;
    }

    Tb_Presence->SetVisibility(ESlateVisibility::Collapsed);
    Th_Loader->SetVisibility(ESlateVisibility::Visible);

    PresenceEssentialsSubsystem->GetPresence(
        PresenceUserId,
        FOnPresenceTaskComplete::CreateWeakLambda(this, [this](const bool bWasSuccessful, const TSharedPtr<FOnlineUserPresenceAccelByte> Presence)
        {
            // Abort if the widget is being destroyed.
            if (!IsValid(this) || IsUnreachable()) 
            {
                return;
            }

            FString PresenceStr;

            // Set offline if presence is invalid.
            if (!bWasSuccessful || !Presence)
            {
                PresenceStr = TEXT_PRESENCE_OFFLINE.ToString();
            }
            // Set valid presence.
            else 
            {
                // Set online and status.
                if (Presence->bIsOnline)
                {
                    PresenceStr = TEXT_PRESENCE_ONLINE.ToString();

                    // Presence status is only be displayed if the presence widget is not displayed in a list entry.
                    if (!ParentListView && 
                        !Presence->Status.StatusStr.IsEmpty() &&
                        !Presence->Status.StatusStr.Equals(FString("nil"), ESearchCase::IgnoreCase))
                    {
                        PresenceStr += FString("\n") + Presence->Status.StatusStr;
                    }
                }
                // Set last seen.
                else
                {
                    PresenceStr = GetLastOnline(Presence->LastOnline);
                }
            }

            // Display presence.
            Th_Loader->SetVisibility(ESlateVisibility::Collapsed);
            Tb_Presence->SetAutoWrapText(ParentListView == nullptr);
            Tb_Presence->SetScrollingEnabled(ParentListView != nullptr);
            Tb_Presence->SetText(FText::FromString(PresenceStr));
            Tb_Presence->SetVisibility(ESlateVisibility::Visible);

            // Refresh list if any.
            if (ParentListView)
            {
                ParentListView->RequestRefresh();
            }

            const FUniqueNetIdAccelByteUserPtr PresenceUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PresenceUserId);
            if (!PresenceUserABId)
            {
                UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Presence widget is updated for user: %s"), *PresenceUserABId->GetAccelByteId());
                return;
            }
        }
    ));
}

void UPresenceWidget::OnPresenceUpdated(const FUniqueNetId& UserId, const TSharedRef<FOnlineUserPresence>& Presence)
{
    const FUniqueNetIdAccelByteUserPtr PresenceUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PresenceUserId);
    if (!PresenceUserABId)
    {
        return;
    }

    if (PresenceUserABId && UserId == PresenceUserId.ToSharedRef().Get())
    {
        UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Received presence update for user: %s. Updating the presence widget."), *PresenceUserABId->GetAccelByteId());
        RefreshPresence();
    }
}

void UPresenceWidget::OnBulkQueryPresenceComplete(const bool bWasSuccessful, const FUserIDPresenceMap& Presences)
{
    const FUniqueNetIdAccelByteUserPtr PresenceUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PresenceUserId);
    if (!PresenceUserABId)
    {
        return;
    }

    if (bWasSuccessful && Presences.Contains(PresenceUserABId->GetAccelByteId()))
    {
        UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Received bulk presence update for user: %s. Updating the presence widget."), *PresenceUserABId->GetAccelByteId());
        RefreshPresence();
    }
}
