// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FindFriendsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/Throbber.h"
#include "CommonButtonBase.h"
#include "HAL/PlatformApplicationMisc.h"

void UFindFriendsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);
}

void UFindFriendsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	Edt_SearchBar->SetText(FText::FromString(TEXT("")));
	Edt_SearchBar->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchBarCommitted);
	Btn_Search->OnClicked().AddWeakLambda(this, [this]()
	{
		OnSearchBarCommitted(Edt_SearchBar->GetText(), ETextCommit::Type::OnEnter);
	});
	Btn_CopyFriendCode->OnClicked().AddUObject(this, &ThisClass::CopyFriendCodeToClipboard);

	// Reset widgets.
	Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_FindFriends->ClearListItems();

	DisplaySelfFriendCode();
}

void UFindFriendsWidget::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();

	Edt_SearchBar->OnTextCommitted.Clear();
	Btn_Search->OnClicked().Clear();
	Btn_CopyFriendCode->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UFindFriendsWidget::NativeGetDesiredFocusTarget() const
{
	return Edt_SearchBar;
}

void UFindFriendsWidget::OnSearchBarCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::Type::OnEnter || Text.IsEmpty())
	{
		return;
	}

	SendFriendRequestByFriendCode(Text.ToString());
}

void UFindFriendsWidget::CopyFriendCodeToClipboard()
{
	FPlatformApplicationMisc::ClipboardCopy(*Tb_FriendCode->GetText().ToString());
}

void UFindFriendsWidget::DisplaySelfFriendCode()
{
	Tb_FriendCode->SetText(FText::GetEmpty());
	Ws_FriendCode->SetActiveWidget(Th_FriendCodeLoader);

	FriendsSubsystem->GetSelfFriendCode(
		GetOwningPlayer(),
		FOnGetSelfFriendCodeComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& FriendCode)
		{
			if (bWasSuccessful && !FriendCode.IsEmpty())
			{
				Tb_FriendCode->SetText(FText::FromString(FriendCode));
				Ws_FriendCode->SetActiveWidget(Tb_FriendCode);
			}
			else
			{
				Ws_FriendCode->SetActiveWidget(Tb_FriendCodeEmpty);
			}
		}
	));
}

void UFindFriendsWidget::FindFriendByDisplayName(const FString& DisplayName)
{
	ensure(FriendsSubsystem);

	Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Try to find friend by exact display name.
	FriendsSubsystem->FindFriend(
		GetOwningPlayer(),
		DisplayName,
		FOnFindFriendComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage)
		{
			Lv_FindFriends->SetUserFocus(GetOwningPlayer());
			Lv_FindFriends->ClearListItems();

			if (bWasSuccessful)
			{
				// Reset the status to be "searched", because the data is retrieved from find friend result.
				FriendData->Status = EFriendStatus::Searched;
				Lv_FindFriends->AddItem(FriendData);
				Lv_FindFriends->RequestRefresh();
				Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
			}
			else
			{
				Ws_FindFriends->ErrorMessage = FText::FromString(ErrorMessage);
				Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
			}
		}
	));
}

void UFindFriendsWidget::SendFriendRequestByFriendCode(const FString& FriendCode)
{
	// Abort if trying to friend with themself.
	if (FriendCode.Equals(Tb_FriendCode->GetText().ToString())) 
	{
		Ws_FindFriends->ErrorMessage = CANNOT_INVITE_FRIEND_SELF;
		Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	ensure(FriendsSubsystem);

	Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Try to send friend request by friend code.
	FriendsSubsystem->SendFriendRequest(
		GetOwningPlayer(),
		FriendCode,
		FOnSendFriendRequestComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage)
		{
			Lv_FindFriends->SetUserFocus(GetOwningPlayer());
			Lv_FindFriends->ClearListItems();

			if (!bWasSuccessful && !FriendData) 
			{
				/* Fallback, the player might enter display name instead of friend code.
				 * Try to find friend by the exact display name. */
				FindFriendByDisplayName(Edt_SearchBar->GetText().ToString());
			}
			else 
			{
				// Reset the status to be "searched", because the data is retrieved from find friend result.
				FriendData->Status = EFriendStatus::Searched;
				Lv_FindFriends->AddItem(FriendData);
				Lv_FindFriends->RequestRefresh();
				Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
			}
		}
	));
}
