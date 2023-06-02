// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UFriendDetailsWidget::InitData(UFriendData* FriendData)
{
	ensure(FriendData);
	CachedFriendData = FriendData;

	Tb_Username->SetText(FText::FromString(CachedFriendData->Username));
	Tb_Presence->SetText(FText::FromString(CachedFriendData->GetPresence()));

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
}