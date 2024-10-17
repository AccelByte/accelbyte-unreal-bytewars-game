// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlockedPlayerWidgetEntry.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UBlockedPlayerWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();

	ManagingFriendsSubsystem = GetGameInstance()->GetSubsystem<UManagingFriendsSubsystem>();
	ensure(ManagingFriendsSubsystem);

	Btn_Unblock->OnClicked().Clear();
	Btn_Unblock->OnClicked().AddUObject(this, &ThisClass::OnUnblockButtonClicked);
}

// @@@SNIPSTART BlockedPlayerWidgetEntry.cpp-NativeOnListItemObjectSet
void UBlockedPlayerWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	CachedBlockedPlayerData = Cast<UFriendData>(ListItemObject);

	// Display display name.
	if (!CachedBlockedPlayerData->DisplayName.IsEmpty())
	{
		Tb_DisplayName->SetText(FText::FromString(CachedBlockedPlayerData->DisplayName));
	}
	else
	{
		Tb_DisplayName->SetText(FText::FromString(
			UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(CachedBlockedPlayerData->UserId.ToSharedRef().Get())));
	}

	// Display avatar image.
	const FString AvatarURL = CachedBlockedPlayerData->AvatarURL;
	Img_Avatar->LoadImage(AvatarURL);

	OnListItemObjectSet.Broadcast();
}
// @@@SNIPEND

// @@@SNIPSTART BlockedPlayerWidgetEntry.cpp-OnUnblockButtonClicked
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "7"]}
void UBlockedPlayerWidgetEntry::OnUnblockButtonClicked()
{
	ensure(CachedBlockedPlayerData);
	ensure(ManagingFriendsSubsystem);

	ManagingFriendsSubsystem->UnblockPlayer(GetOwningPlayer(), CachedBlockedPlayerData->UserId);
}
// @@@SNIPEND

#undef LOCTEXT_NAMESPACE