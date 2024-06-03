// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendDetailsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendDetailsWidget::InitData(UFriendData* FriendData)
{
	if (!FriendData) 
	{
		UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Unable to display friend details. Friend data is not valid."));
		return;
	}
	
	CachedFriendData = FriendData;

	// Display display name.
	if (!CachedFriendData->DisplayName.IsEmpty())
	{
		Tb_DisplayName->SetText(FText::FromString(CachedFriendData->DisplayName));
	}
	else
	{
		Tb_DisplayName->SetText(FText::FromString(
			UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(CachedFriendData->UserId.ToSharedRef().Get())));
	}

	// Display avatar image.
	const FString AvatarURL = CachedFriendData->AvatarURL;
	Img_Avatar->LoadImage(AvatarURL);
}

#undef LOCTEXT_NAMESPACE