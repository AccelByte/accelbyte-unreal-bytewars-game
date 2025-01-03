// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsPlatformWidget.h"

#include "CommonUILibrary.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByte.h"
#include "Access/AuthEssentials/AuthEssentialsModels.h"
#include "Components/ListView.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/FriendsEssentials/UI/FriendWidgetEntry.h"
#include "Social/FriendsEssentials/UI/FriendWidgetEntry_Starter.h"
#include "Social/ManagingFriends/UI/BlockedPlayerWidgetEntry.h"
#include "Social/ManagingFriends/UI/BlockedPlayerWidgetEntry_Starter.h"
#include "Storage/StatisticsEssentials/UI/ProfileWidget.h"

void UAccelByteWarsPlatformWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	FUniqueNetIdPtr NetIdPtr;

	Lv_Icon->ClearListItems();

	// Get User ID from parent widget
	if (const UPlayerEntryWidget* PlayerEntryWidget =
		Cast<UPlayerEntryWidget>(UCommonUILibrary::FindParentWidgetOfType(this, UPlayerEntryWidget::StaticClass())))
	{
		NetIdPtr = PlayerEntryWidget->GetNetId();
	}
	else if (const UProfileWidget* ProfileWidget =
		Cast<UProfileWidget>(UCommonUILibrary::FindParentWidgetOfType(this, UProfileWidget::StaticClass())))
	{
		bShowAll = true;
		NetIdPtr = ProfileWidget->GetNetId();
	}
	else if (const UFriendDetailsWidget* FriendDetailsWidget =
		Cast<UFriendDetailsWidget>(UCommonUILibrary::FindParentWidgetOfType(this, UFriendDetailsWidget::StaticClass())))
	{
		bShowAll = true;
		NetIdPtr = FriendDetailsWidget->GetCachedFriendData()->UserId;
	}
	else if (UAccelByteWarsWidgetEntry* WidgetEntry =
		Cast<UAccelByteWarsWidgetEntry>(UCommonUILibrary::FindParentWidgetOfType(this, UAccelByteWarsWidgetEntry::StaticClass())))
	{
		// Entry list will have its data set on its ListItemObjectSet. Change the flow to after that instead
		WidgetEntry->GetOnListItemObjectSet()->AddUObject(this, &ThisClass::SetupByWidgetEntry, WidgetEntry);
		return;
	}

	// Hide widget if there's no ID found
	if (!NetIdPtr.IsValid())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Setup(NetIdPtr);
}

void UAccelByteWarsPlatformWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (UAccelByteWarsWidgetEntry* WidgetEntry =
		Cast<UAccelByteWarsWidgetEntry>(UCommonUILibrary::FindParentWidgetOfType(this, UAccelByteWarsWidgetEntry::StaticClass())))
	{
		WidgetEntry->GetOnListItemObjectSet()->RemoveAll(this);
	}
}

void UAccelByteWarsPlatformWidget::SetupByWidgetEntry(UAccelByteWarsWidgetEntry* WidgetEntry)
{
	FUniqueNetIdPtr NetIdPtr;

	if (const UFriendWidgetEntry* FriendWidget = Cast<UFriendWidgetEntry>(WidgetEntry))
	{
		if (const UFriendData* FriendData = FriendWidget->GetCachedFriendData())
		{
			NetIdPtr = FriendData->UserId;
		}
	}
	else if (const UFriendWidgetEntry_Starter* FriendWidget_Starter = Cast<UFriendWidgetEntry_Starter>(WidgetEntry))
	{
		if (const UFriendData* FriendData = FriendWidget_Starter->GetCachedFriendData())
		{
			NetIdPtr = FriendData->UserId;
		}
	}
	else if (const UBlockedPlayerWidgetEntry* BlockedPlayerWidget = Cast<UBlockedPlayerWidgetEntry>(WidgetEntry))
	{
		if (const UFriendData* FriendData = BlockedPlayerWidget->GetCachedBlockedFriendData())
		{
			NetIdPtr = FriendData->UserId;
		}
	}
	else if (const UBlockedPlayerWidgetEntry_Starter* BlockedPlayerWidget_Starter = Cast<UBlockedPlayerWidgetEntry_Starter>(WidgetEntry))
	{
		if (const UFriendData* FriendData = BlockedPlayerWidget_Starter->GetCachedBlockedFriendData())
		{
			NetIdPtr = FriendData->UserId;
		}
	}

	// Hide widget if there's no ID found
	if (!NetIdPtr.IsValid())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Setup(NetIdPtr);
}

void UAccelByteWarsPlatformWidget::Setup(const FUniqueNetIdPtr NetIdPtr)
{
	TArray<UPlatformWidgetData*> PlatformWidgetDatas;

	// Construct list item data
	// Check if UserId have its platform type
	if (UAuthEssentialsModels::GetPlatformType(NetIdPtr) == EAccelBytePlatformType::None)
	{
		// No platform type, check user info
		if (const FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld())))
		{
			const TSharedPtr<const FAccelByteUserInfo> UserInfo = Subsystem->GetUserCache()->GetUser(*NetIdPtr.Get());
			if (bShowAll)
			{
				for (const FAccelByteLinkedUserInfo& LinkedPlatformInfo : UserInfo->LinkedPlatformInfo)
				{
					PlatformWidgetDatas.Add(UserIdToPlatformWidgetData(LinkedPlatformInfo.Id));
				}
			}
			else if (!UserInfo->LinkedPlatformInfo.IsEmpty())
			{
				PlatformWidgetDatas.Add(UserIdToPlatformWidgetData(UserInfo->LinkedPlatformInfo[0].Id));
			}
		}

		// If current user ID's platform type is none and there's no Linked Platform info, destroy widget
		if (PlatformWidgetDatas.IsEmpty())
		{
			RemoveFromParent();
			return;
		}
	}
	else
	{
		PlatformWidgetDatas.Add(UserIdToPlatformWidgetData(NetIdPtr));
	}

	Lv_Icon->SetListItems(PlatformWidgetDatas);
}

UPlatformWidgetData* UAccelByteWarsPlatformWidget::UserIdToPlatformWidgetData(const FUniqueNetIdPtr NetId) const
{
	UPlatformWidgetData* PlatformWidgetData = NewObject<UPlatformWidgetData>();
	PlatformWidgetData->Init(NetId);

	return PlatformWidgetData;
}
