// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Social/PrivateChat/PrivateChatSubsystem_Starter.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PrivateChatWidget_Starter.generated.h"

class UCommonButtonBase;
class UPromptSubsystem;
class UChatWidget;

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
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

#pragma region Module Private Chat Function Declarations
	
	// TODO: Add your Module Private Chat function declarations here.
	
#pragma endregion

	UPROPERTY()
	UPrivateChatSubsystem_Starter* PrivateChatSubsystem;

	UPROPERTY()
	UPromptSubsystem* PromptSubsystem;

	FUniqueNetIdPtr PrivateChatRecipientUserId = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UChatWidget* W_Chat;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
};
