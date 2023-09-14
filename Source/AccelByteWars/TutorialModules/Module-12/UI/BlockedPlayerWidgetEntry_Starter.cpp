// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-12/UI/BlockedPlayerWidgetEntry_Starter.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TutorialModules/Module-1/TutorialModuleOnlineUtility.h"

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

	// Store default brush to be used to reset the avatar brush if needed.
	if (!DefaultAvatarBrush.GetResourceObject())
	{
		DefaultAvatarBrush = Img_Avatar->Brush;
	}

	// Display avatar image.
	const FString AvatarURL = CachedBlockedPlayerData->AvatarURL;
	const FString AvatarId = FBase64::Encode(AvatarURL);

	// Try to set avatar image from cache.
	FCacheBrush CacheAvatarBrush = AccelByteWarsUtility::GetImageFromCache(AvatarId);
	if (CacheAvatarBrush.IsValid())
	{
		Img_Avatar->SetBrush(*CacheAvatarBrush.Get());
	}
	// Set avatar image from URL if it is not exists in cache.
	else if (!AvatarURL.IsEmpty())
	{
		AccelByteWarsUtility::GetImageFromURL(
			AvatarURL,
			AvatarId,
			FOnImageReceived::CreateWeakLambda(this, [this](const FCacheBrush ImageResult)
			{
				Img_Avatar->SetBrush(*ImageResult.Get());
			})
		);
	}
	// If no valid avatar, reset it to the default one.
	else
	{
		Img_Avatar->SetBrush(DefaultAvatarBrush);
	}
}

void UBlockedPlayerWidgetEntry_Starter::OnUnblockButtonClicked()
{
	// TODO: Call unblock player request here.
}

#undef LOCTEXT_NAMESPACE