// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"

#include "ChatWidget.generated.h"

class UEditableText;
class UListView;
class UCommonButtonBase;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UChatWidget : public UCommonUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

// @@@SNIPSTART ChatWidget.h-public
// @@@MULTISNIP Overview {"selectedLines": ["1", "6", "11", "16", "22", "27", "32"]}
public:
	/**
	 * @brief Add chat entry
	 * @param ChatData Data to append
	 */
	void AppendChatMessage(UChatData* ChatData) const;

	/**
	 * @brief Clear all chat entries
	 */
	void ClearChatMessages() const;

	/**
	 * @brief Clear input text and related widget components
	 */
	void ClearInput() const;

	/**
	 * @brief Change current widget state
	 * @param State Target state
	 */
	void SetWidgetState(const EAccelByteWarsWidgetSwitcherState State) const;

	/**
	 * @brief Retrieve predefined max chat entry
	 */
	int32 GetMaxChatHistory() const { return MaxChatHistory; }

	/**
	 * @brief Triggered on enter on input text or on click send button
	 */
	FChatOnSubmit OnSubmitDelegates;
// @@@SNIPEND

// @@@SNIPSTART ChatWidget.h-protected
// @@@MULTISNIP Vars {"selectedLines": ["1-10"]}
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxChatHistory = 10000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SendChatCooldown = 1.0f;

	// Based on the default value of chat character limit on Admin Portal
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxMessageLength = 500;
// @@@SNIPEND

// @@@SNIPSTART ChatWidget.h-private
// @@@MULTISNIP Overview {"selectedLines": ["1", "13-23"]}
private:
	UFUNCTION()
	void OnChatMessageChanged(const FText& Text);

	UFUNCTION()
	void OnChatMessageSubmit(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void SubmitChat();

	void UpdateCharacterCount(const int32 CharacterCount) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_ChatMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Send;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Character_Max;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Character_Count;

	FTimerHandle SendChatDelayTimerHandle;
// @@@SNIPEND
};
