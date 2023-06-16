// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FriendWidgetEntry_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendWidgetEntry_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem_Starter>();
	ensure(FriendsSubsystem);

	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::OnInviteButtonClicked);
	Btn_Accept->OnClicked().AddUObject(this, &ThisClass::OnAcceptButtonClicked);
	Btn_Reject->OnClicked().AddUObject(this, &ThisClass::OnRejectButtonClicked);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::OnCancelButtonClicked);
}

void UFriendWidgetEntry_Starter::NativeOnListItemObjectSet(UObject* ListItemObject)
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

	// Display options based on friend's invitation status.
	Ws_OptionButtons->SetActiveWidgetIndex((uint8)CachedFriendData->Status);

	// Show the reason why the player cannot send invitation request.
	Btn_Invite->SetVisibility(!CachedFriendData->bCannotBeInvited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Tb_CannotInviteMessage->SetVisibility(CachedFriendData->bCannotBeInvited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Tb_CannotInviteMessage->SetText(FText::FromString(CachedFriendData->ReasonCannotBeInvited));
}

void UFriendWidgetEntry_Starter::OnInviteButtonClicked()
{
	// TODO: Call send friend request here.
}

void UFriendWidgetEntry_Starter::OnAcceptButtonClicked()
{
	// TODO: Call accept friend request here.
}

void UFriendWidgetEntry_Starter::OnRejectButtonClicked()
{
	// TODO: Call reject friend request here.
}

void UFriendWidgetEntry_Starter::OnCancelButtonClicked()
{
	// TODO: Call cancel friend request here.
}

#undef LOCTEXT_NAMESPACE