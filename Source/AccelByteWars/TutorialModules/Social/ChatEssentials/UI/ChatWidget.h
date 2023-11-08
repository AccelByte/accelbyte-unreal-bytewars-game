// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/ChatEssentials/ChatEssentialsSubsystem.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "ChatWidget.generated.h"

class UWidgetSwitcher;
class UListView;
class UEditableText;
class UTextBlock;
class UCommonButtonBase;
class UVerticalBox;
class UPromptSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API UChatWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void SwitchChatMessageType(const EAccelByteChatRoomType ChatRoomType);
	UListView* GetChatMessageContainerBasedOnChatRoomType(const EAccelByteChatRoomType ChatRoomType);

	void AppendChatMessage(UChatData* ChatData, const EAccelByteChatRoomType ChatRoomType);
	void AppendChatMessage(const FChatMessage& Message, const EAccelByteChatRoomType ChatRoomType);

	void SendChatMessage();
	
	UFUNCTION()
	void OnSendChatMessageCommited(const FText& Text, ETextCommit::Type CommitMethod);

	void GetLastChatMessages(const EAccelByteChatRoomType ChatRoomType);
	
	void OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnChatRoomMessageReceived(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	UChatEssentialsSubsystem* ChatEssentialsSubsystem;
	UPromptSubsystem* PromptSubsystem;

	EAccelByteChatRoomType CurrentChatRoomType = EAccelByteChatRoomType::NORMAL;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ChatMessageType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_SessionChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_SessionChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_PartyChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_PartyChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_SessionChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_PartyChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Send;
};
