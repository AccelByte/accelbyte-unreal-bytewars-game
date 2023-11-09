// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/PrivateChat/PrivateChatSubsystem.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PrivateChatWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UListView;
class UEditableText;
class UCommonButtonBase;
class UPromptSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPrivateChatWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void SetPrivateChatRecipient(FUniqueNetIdPtr RecipientUserId);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void AppendChatMessage(UChatData* ChatData);
	void AppendChatMessage(const FChatMessage& Message);

	void SendPrivateChatMessage();

	UFUNCTION()
	void OnSendPrivateChatMessageCommited(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnChatMessageChanged(const FText& Text);

	void GetLastPrivateChatMessages();

	void OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnPrivateChatMessageReceived(const FUniqueNetId& Sender, const TSharedRef<FChatMessage>& Message);

	UPrivateChatSubsystem* PrivateChatSubsystem;
	UPromptSubsystem* PromptSubsystem;

	FUniqueNetIdPtr PrivateChatRecipientUserId = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SendChatCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxMessageLength = 80;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxChatHistory = 10000;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Send;

	FTimerHandle SendChatDelayTimerHandle;
};
