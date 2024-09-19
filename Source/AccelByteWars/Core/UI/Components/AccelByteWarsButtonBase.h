// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Core/UI/AccelByteWarsWidgetInterface.h"
#include "AccelByteWarsButtonBase.generated.h"

UENUM(BlueprintType)
enum class EAccelByteWarsButtonBaseType : uint8
{
	TEXT_BUTTON UMETA(DisplayName = "Text Button"),
	IMAGE_BUTTON UMETA(DisplayName = "Image Button")
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsButtonBase : public UCommonButtonBase, public IAccelByteWarsWidgetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetButtonText(const FText& InText);

	UFUNCTION(BlueprintCallable)
	void SetButtonImage(const FSlateBrush& InBrush);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetButtonType(const EAccelByteWarsButtonBaseType& InType);

	UFUNCTION(BlueprintCallable)
	void SetAllowClickSound(bool bAllow);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ToggleExclamationMark (const bool bShowMark);
	
	UFUNCTION(BlueprintCallable)
	void ClearButtonBindings();

	UFUNCTION(BlueprintCallable)
	void CallOnClick();

protected:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	// End of UUserWidget interface

	// UCommonButtonBase interface
	virtual void UpdateInputActionWidget() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	virtual void NativeOnHovered() override;
	// End of UCommonButtonBase interface

	void RefreshButtonText();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonText(const FText& InText);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonStyle();

	void RefreshButtonImage();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonImage(const FSlateBrush& InBrush);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	EAccelByteWarsButtonBaseType ButtonType = EAccelByteWarsButtonBaseType::TEXT_BUTTON;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	bool bShowExclamationMark = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	bool bSelectedByKeyboard = false;
	
private:
	UPROPERTY(EditAnywhere, Category = "Button", meta = (EditCondition = "ButtonType==EAccelByteWarsButtonBaseType::TEXT_BUTTON", EditConditionHides))
	uint8 bOverride_ButtonText : 1;

	UPROPERTY(EditAnywhere, Category = "Button", meta = (EditCondition = "ButtonType==EAccelByteWarsButtonBaseType::TEXT_BUTTON", EditConditionHides))
	FText ButtonText;

	UPROPERTY(EditAnywhere, Category = "Button", meta = (EditCondition = "ButtonType==EAccelByteWarsButtonBaseType::IMAGE_BUTTON", EditConditionHides))
	FSlateBrush ButtonBrush;

	UPROPERTY(EditAnywhere, Category = "Button")
	bool bAllowClickSound = true;

	UPROPERTY()
	FSlateSound ClickSound;
};
