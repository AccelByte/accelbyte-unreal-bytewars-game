// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativeFriendWidgetEntry.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void UNativeFriendWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();

	FriendsSubsystem = GetGameInstance()->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);

	NativeFriendsSubsystem = GetGameInstance()->GetSubsystem<UNativeFriendsSubsystem>();
	ensure(NativeFriendsSubsystem);

	Btn_Invite->OnClicked().Clear();
	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::OnInviteButtonClicked);
}

void UNativeFriendWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
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

void UNativeFriendWidgetEntry::OnInviteButtonClicked()
{
	ensure(NativeCachedFriendData);
	ensure(FriendsSubsystem);

	FriendsSubsystem->SendFriendRequest(
		GetOwningPlayer(), 
		NativeCachedFriendData->UserId,
		FOnSendFriendRequestComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage) 
		{
			if (bWasSuccessful) 
			{
				// Since the invitation is already sent, refresh the entry data to show that the friend cannot be invited again.
				NativeCachedFriendData->Status = ENativeFriendStatus::PendingOutbound;
				NativeCachedFriendData->ReasonCannotBeInvited = ALREADY_INVITED_REASON_MESSAGE.ToString();
				NativeOnListItemObjectSet(NativeCachedFriendData);
			}
		}
	));
}
