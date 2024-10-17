// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendDetailsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Social/ManagingFriends/ManagingFriendsSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendDetailsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
	
	UManagingFriendsSubsystem* ManagingFriendsSubsystem = GameInstance->GetSubsystem<UManagingFriendsSubsystem>();
	if(ManagingFriendsSubsystem != nullptr)
	{
		ManagingFriendsSubsystem->BindGeneratedWidgetButtonsAction(GetOwningPlayer(), CachedFriendData);
		ManagingFriendsSubsystem->SetGeneratedWidgetButtonsVisibility(CachedFriendData);
	}
}

void UFriendDetailsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
	
	UManagingFriendsSubsystem* ManagingFriendsSubsystem = GameInstance->GetSubsystem<UManagingFriendsSubsystem>();
	if(ManagingFriendsSubsystem != nullptr)
	{
		ManagingFriendsSubsystem->UnbindGeneratedWidgetButtonsAction();
	}
}

// @@@SNIPSTART FriendDetailsWidget.cpp-InitData
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
// @@@SNIPEND

#undef LOCTEXT_NAMESPACE