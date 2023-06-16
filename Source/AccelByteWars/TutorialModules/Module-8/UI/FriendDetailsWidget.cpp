// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendDetailsWidget::InitData(UFriendData* FriendData)
{
	ensure(FriendData);
	CachedFriendData = FriendData;

	// Display display name.
	if (!CachedFriendData->DisplayName.IsEmpty())
	{
		Tb_DisplayName->SetText(FText::FromString(CachedFriendData->DisplayName));
	}
	else
	{
		Tb_DisplayName->SetText(LOCTEXT("Byte Wars Player", "Byte Wars Player"));
	}

	// Display presence.
	Tb_Presence->SetText(FText::FromString(CachedFriendData->GetPresence()));

	// Store default brush to be used to reset the avatar brush if needed.
	if (!DefaultAvatarBrush.GetResourceObject())
	{
		DefaultAvatarBrush = Img_Avatar->Background;
	}

	// Display avatar image.
	const FString AvatarURL = CachedFriendData->AvatarURL;
	const FString AvatarId = FBase64::Encode(AvatarURL);

	// Try to set avatar image from cache.
	FCacheBrush CacheAvatarBrush = AccelByteWarsUtility::GetImageFromCache(AvatarId);
	if (CacheAvatarBrush.IsValid())
	{
		Img_Avatar->SetBrushColor(FLinearColor::White);
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
				Img_Avatar->SetBrushColor(FLinearColor::White);
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

#undef LOCTEXT_NAMESPACE