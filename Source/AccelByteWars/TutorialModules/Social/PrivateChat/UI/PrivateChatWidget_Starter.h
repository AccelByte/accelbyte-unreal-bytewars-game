// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/PrivateChat/PrivateChatSubsystem_Starter.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PrivateChatWidget_Starter.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UListView;
class UEditableText;
class UCommonButtonBase;
class UPromptSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API UPrivateChatWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void SetPrivateChatRecipient(FUniqueNetIdPtr RecipientUserId);
	FUniqueNetIdPtr GetPrivateChatRecipient() const { return PrivateChatRecipientUserId; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
	UFUNCTION()
	void OnChatMessageChanged(const FText& Text);

#pragma region Module Private Chat Function Declarations
	
	// TODO: Add your Module Private Chat function declarations here.
	
#pragma endregion

	UPrivateChatSubsystem_Starter* PrivateChatSubsystem;
	UPromptSubsystem* PromptSubsystem;

	FUniqueNetIdPtr PrivateChatRecipientUserId = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SendChatCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxMessageLength = 100;

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
