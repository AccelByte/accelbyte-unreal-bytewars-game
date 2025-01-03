// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/PrivateChat/PrivateChatSubsystem.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PrivateChatWidget.generated.h"

class UCommonButtonBase;
class UPromptSubsystem;
class UChatWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPrivateChatWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void SetPrivateChatRecipient(FUniqueNetIdPtr RecipientUserId);
	FUniqueNetIdPtr GetPrivateChatRecipient() const { return PrivateChatRecipientUserId; }

// @@@SNIPSTART PrivateChatWidget.h-protected
// @@@MULTISNIP Overview {"selectedLines": ["1", "25-26"]}
// @@@MULTISNIP AddingUI {"selectedLines": ["1", "7-15"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void AppendChatMessage(const FChatMessage& Message) const;

	UFUNCTION()
	void SendPrivateChatMessage(const FText& MessageText);
	void OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);

	void OnPrivateChatMessageReceived(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message);

	void GetLastPrivateChatMessages() const;

	UPROPERTY()
	UPrivateChatSubsystem* PrivateChatSubsystem;

	UPROPERTY()
	UPromptSubsystem* PromptSubsystem;

	FUniqueNetIdPtr PrivateChatRecipientUserId = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UChatWidget* W_Chat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
// @@@SNIPEND
};
