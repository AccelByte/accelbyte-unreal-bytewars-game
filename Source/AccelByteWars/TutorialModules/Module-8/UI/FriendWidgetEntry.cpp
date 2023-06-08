// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FriendWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);

	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::OnInviteButtonClicked);
	Btn_Accept->OnClicked().AddUObject(this, &ThisClass::OnAcceptButtonClicked);
	Btn_Reject->OnClicked().AddUObject(this, &ThisClass::OnRejectButtonClicked);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::OnCancelButtonClicked);
	Btn_Unblock->OnClicked().AddUObject(this, &ThisClass::OnUnblockButtonClicked);
}

void UFriendWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	CachedFriendData = Cast<UFriendData>(ListItemObject);

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

	// Display options based on friend's invitation status.
	Ws_OptionButtons->SetActiveWidgetIndex((uint8)CachedFriendData->Status);

	// Show the reason why the player cannot send invitation request.
	Btn_Invite->SetVisibility(!CachedFriendData->bCannotBeInvited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Tb_CannotInviteMessage->SetVisibility(CachedFriendData->bCannotBeInvited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Tb_CannotInviteMessage->SetText(FText::FromString(CachedFriendData->ReasonCannotBeInvited));
}

void UFriendWidgetEntry::OnInviteButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	FriendsSubsystem->SendFriendRequest(
		GetOwningPlayer(), 
		CachedFriendData->UserId, 
		FOnSendFriendRequestComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage) 
		{
			if (bWasSuccessful) 
			{
				// Since the invitation is already sent, refresh the entry data to show that the friend cannot be invited again.
				FriendData->Status = EFriendStatus::Searched;
				NativeOnListItemObjectSet(FriendData);
			}
		}
	));
}

void UFriendWidgetEntry::OnAcceptButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	FriendsSubsystem->AcceptFriendRequest(GetOwningPlayer(), CachedFriendData->UserId);
}

void UFriendWidgetEntry::OnRejectButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	FriendsSubsystem->RejectFriendRequest(GetOwningPlayer(), CachedFriendData->UserId);
}

void UFriendWidgetEntry::OnCancelButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	// Cancel friend request is the same as removing a friend.
	FriendsSubsystem->RemoveFriend(GetOwningPlayer(), CachedFriendData->UserId);
}

void UFriendWidgetEntry::OnUnblockButtonClicked()
{
	// TODO: Implement unblock player on the future tutorial module.
}

#undef LOCTEXT_NAMESPACE