// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/SessionChat/SessionChatSubsystem.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "SessionChatWidget.generated.h"

class UWidgetSwitcher;
class UPromptSubsystem;
class UCommonButtonBase;
class UChatWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API USessionChatWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void SetDefaultChatType(const EAccelByteChatRoomType ChatRoomType);
	EAccelByteChatRoomType GetCurrentChatType() const { return CurrentChatRoomType; }

// @@@SNIPSTART SessionChatWidget.h-protected
// @@@MULTISNIP Overview {"selectedLines": ["1", "24-55"]}
// @@@MULTISNIP AddingUI {"selectedLines": ["1", "9-16"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void SwitchChatMessageType(const EAccelByteChatRoomType ChatRoomType);

	void AppendChatMessage(const FChatMessage& Message);

	void SendChatMessage(const FText& MessageText);

	void GetLastChatMessages();
	
	void OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnChatRoomMessageReceived(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	UPROPERTY()
	USessionChatSubsystem* SessionChatSubsystem;

	UPROPERTY()
	UPromptSubsystem* PromptSubsystem;

	EAccelByteChatRoomType CurrentChatRoomType = EAccelByteChatRoomType::SESSION_V2;

	UPROPERTY()
	UChatWidget* W_ActiveChat = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ChatMessageType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

#pragma region Game Session Chat
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_GameSessionChatOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UChatWidget* W_GameSessionChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_SessionChat;
#pragma endregion

#pragma region Party Chat
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_PartyChatOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UChatWidget* W_PartyChat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_PartyChat;
#pragma endregion
// @@@SNIPEND
};
