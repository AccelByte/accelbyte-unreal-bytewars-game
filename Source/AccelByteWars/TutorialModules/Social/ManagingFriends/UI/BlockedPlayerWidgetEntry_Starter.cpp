// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlockedPlayerWidgetEntry_Starter.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UBlockedPlayerWidgetEntry_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	ManagingFriendsSubsystem = GetGameInstance()->GetSubsystem<UManagingFriendsSubsystem_Starter>();
	ensure(ManagingFriendsSubsystem);

	Btn_Unblock->OnClicked().Clear();
	Btn_Unblock->OnClicked().AddUObject(this, &ThisClass::OnUnblockButtonClicked);
}

void UBlockedPlayerWidgetEntry_Starter::NativeOnListItemObjectSet(UObject* ListItemObject)
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

void UBlockedPlayerWidgetEntry_Starter::OnUnblockButtonClicked()
{
	// TODO: Call unblock player request here.
}

#undef LOCTEXT_NAMESPACE