// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativeFriendWidgetEntry_Starter.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void UNativeFriendWidgetEntry_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	FriendsSubsystem = GetGameInstance()->GetSubsystem<UFriendsSubsystem_Starter>();
	ensure(FriendsSubsystem);

	NativeFriendsSubsystem = GetGameInstance()->GetSubsystem<UNativeFriendsSubsystem_Starter>();
	ensure(NativeFriendsSubsystem);

	Btn_Invite->OnClicked().Clear();
	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::OnInviteButtonClicked);
}

void UNativeFriendWidgetEntry_Starter::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	NativeCachedFriendData = Cast<UNativeFriendData>(ListItemObject);

	// Display display name.
	Tb_DisplayName->SetText(NativeCachedFriendData->DisplayName.IsEmpty() ? FText::FromString(NativeCachedFriendData->DisplayName)
		: FText::FromString(UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(NativeCachedFriendData->UserId.ToSharedRef().Get())));

	// Display avatar image.
	const FString AvatarURL = NativeCachedFriendData->AvatarURL;
	Img_Avatar->LoadImage(AvatarURL);

	// Display options based on friend's status.
	switch (NativeCachedFriendData->Status)
	{
	case ENativeFriendStatus::Unknown:
		Ws_Option->SetActiveWidget(Btn_Invite);
		break;
	default:
		Ws_Option->SetActiveWidget(Tb_Message);
		Tb_Message->SetText(FText::FromString(NativeCachedFriendData->ReasonCannotBeInvited));
		break;
	}

	OnListItemObjectSet.Broadcast();
}

void UNativeFriendWidgetEntry_Starter::OnInviteButtonClicked()
{
	// TODO: Call send friend request here.
}
