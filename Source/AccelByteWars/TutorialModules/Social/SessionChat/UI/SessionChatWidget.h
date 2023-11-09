// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/SessionChat/SessionChatSubsystem.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "SessionChatWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UWidgetSwitcher;
class UListView;
class UEditableText;
class UCommonButtonBase;
class UVerticalBox;
class UPromptSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API USessionChatWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void SetDefaultChatType(const EAccelByteChatRoomType ChatRoomType);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void SwitchChatMessageType(const EAccelByteChatRoomType ChatRoomType);

	void AppendChatMessage(UChatData* ChatData);
	void AppendChatMessage(const FChatMessage& Message);

	void SendChatMessage();
	
	UFUNCTION()
	void OnSendChatMessageCommited(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnChatMessageChanged(const FText& Text);

	void GetLastChatMessages();
	
	void OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnChatRoomMessageReceived(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	USessionChatSubsystem* SessionChatSubsystem;
	UPromptSubsystem* PromptSubsystem;

	EAccelByteChatRoomType CurrentChatRoomType = EAccelByteChatRoomType::SESSION_V2;

	UListView* Lv_ChatMessage = nullptr;
	UAccelByteWarsWidgetSwitcher* Ws_ChatMessage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SendChatCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxMessageLength = 80;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxChatHistory = 10000;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ChatMessageType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ChatMessageTypeButton;

#pragma region Game Session Chat
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_GameSessionChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_GameSessionChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_GameSessionChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_SessionChat;
#pragma endregion

#pragma region Party Chat
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UVerticalBox* Vb_PartyChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_PartyChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_PartyChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_PartyChat;
#pragma endregion

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Send;

	FTimerHandle SendChatDelayTimerHandle;
};
