// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChatWidget.h"

#include "Components/ListView.h"
#include "CommonButtonBase.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Delegate setup
	Btn_Send->OnClicked().AddUObject(this, &ThisClass::SubmitChat);
	Edt_ChatMessage->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnChatMessageSubmit);
	Edt_ChatMessage->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnChatMessageChanged);

	ClearInput();
}

void UChatWidget::NativeDestruct()
{
	// Delegate cleanup
	Btn_Send->OnClicked().RemoveAll(this);
	Edt_ChatMessage->OnTextCommitted.RemoveAll(this);
	Edt_ChatMessage->OnTextChanged.RemoveAll(this);

	Super::NativeDestruct();
}

FReply UChatWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	Edt_ChatMessage->SetUserFocus(GetOwningPlayer());
	return FReply::Handled();
}

void UChatWidget::AppendChatMessage(UChatData* ChatData) const
{
	// Safety
	if (!Lv_ChatMessage || !ChatData)
	{
		return;
	}

	// Display chat message.
	Lv_ChatMessage->AddItem(ChatData);
	Lv_ChatMessage->ScrollToBottom();
	SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
}

void UChatWidget::ClearChatMessages() const
{
	// Safety
	if (!Lv_ChatMessage)
	{
		return;
	}

	Lv_ChatMessage->ClearListItems();
}

void UChatWidget::ClearInput() const
{
	Edt_ChatMessage->SetText(FText::GetEmpty());
	UpdateCharacterCount(0);
	Tb_Character_Max->SetVisibility(ESlateVisibility::Collapsed);
	Btn_Send->SetIsEnabled(false);

	// Change focus
	Edt_ChatMessage->SetUserFocus(GetOwningPlayer());
}

void UChatWidget::SetWidgetState(const EAccelByteWarsWidgetSwitcherState State, const FText& StateMessage) const
{
	if (!StateMessage.IsEmpty()) 
	{
		switch (State) 
		{
		case EAccelByteWarsWidgetSwitcherState::Empty:
			Ws_ChatMessage->EmptyMessage = StateMessage;
			break;
		case EAccelByteWarsWidgetSwitcherState::Loading:
			Ws_ChatMessage->LoadingMessage = StateMessage;
			break;
		case EAccelByteWarsWidgetSwitcherState::Error:
			Ws_ChatMessage->ErrorMessage = StateMessage;
			break;
		}
	}
	Ws_ChatMessage->SetWidgetState(State);
}

void UChatWidget::OnChatMessageChanged(const FText& Text)
{
	// Disable the send button if it is empty.
	Btn_Send->SetIsEnabled(!Text.IsEmpty());

	// Limit the chat message length.
	const FText ModifiedText = FText::FromString(Edt_ChatMessage->GetText().ToString().Left(MaxMessageLength));
	Edt_ChatMessage->SetText(ModifiedText);

	// Update word count
	const int32 CharacterCount = ModifiedText.ToString().Len();
	UpdateCharacterCount(CharacterCount);
	Tb_Character_Max->SetVisibility(CharacterCount >= MaxMessageLength ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UChatWidget::OnChatMessageSubmit(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	SubmitChat();
}

void UChatWidget::SubmitChat()
{
	// Refresh text pointer.
	Edt_ChatMessage->SetText(Edt_ChatMessage->GetText());

	// Don't submit empty message.
	const FText Chat = Edt_ChatMessage->GetText();
	if (Chat.IsEmpty())
	{
		return;
	}

	// Submit
	OnSubmitDelegates.Broadcast(Chat);

	// Clear text after sending chat.
	ClearInput();

	// Cooldown to prevent spamming.
	Edt_ChatMessage->SetIsEnabled(false);
	Btn_Send->SetIsEnabled(false);
	GetGameInstance()->GetTimerManager().SetTimer(
		SendChatDelayTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			Edt_ChatMessage->SetIsEnabled(true);
			Btn_Send->SetIsEnabled(true);
			ClearInput();
		}),
		SendChatCooldown, false, SendChatCooldown);
}

void UChatWidget::UpdateCharacterCount(const int32 CharacterCount) const
{
	Tb_Character_Count->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CharacterCount, MaxMessageLength)));
}
