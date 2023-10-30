// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserListEntry.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "OptionListEntryBase.generated.h"

class UAnalogSlider;
class USlider;
class UImage;
class UCommonTextBlock;
class UCommonButtonBase;
class UCommonRotator;
class UCheckBox;

UENUM(BlueprintType)
enum class EOptionEntryType : uint8
{
	EntryScalar,
	EntryDiscrete
};

USTRUCT(BlueprintType)
struct FOptionEntryDiscreteValue
{
	GENERATED_USTRUCT_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString DefaultValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Value;
};

UCLASS(Abstract, NotBlueprintable)
class ACCELBYTEWARS_API UOptionListEntryBase : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void SetDisplayName(const FText& InName);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	FText DisplayName = FText::GetEmpty();
	
protected:
	virtual void NativeOnEntryReleased() override;
	
	// Focus transitioning to subwidgets for the gamepad
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	UFUNCTION(BlueprintImplementableEvent)
	UWidget* GetPrimaryGamepadFocusWidget();

	EOptionEntryType EntryType;
};

//////////////////////////////////////////////////////////////////////////
// UOptionListEntry_Scalar
//////////////////////////////////////////////////////////////////////////

DECLARE_MULTICAST_DELEGATE_OneParam(FOnScalarValueChangedDelegate, float /*Value*/);

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API UOptionListEntry_Scalar : public UOptionListEntryBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitOption(const FText& InName, const float InValue);

	void SetDisplayName(const FText& InName) override;

	UFUNCTION(BlueprintCallable)
	void SetScalarValue(const float InValue);

	UFUNCTION(BlueprintPure)
	float GetScalarValue();

	FOnScalarValueChangedDelegate OnScalarValueChangedDelegate;
	
protected:
	void NativeOnInitialized() override;

	UFUNCTION()
	void OnScalarValueChanged(float Value);

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_ProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAnalogSlider* Slider_OptionValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_OptionValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_OptionName;
};

//////////////////////////////////////////////////////////////////////////
// UOptionListEntry_Discrete
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API UOptionListEntry_Discrete : public UOptionListEntryBase
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintCallable)
	void Refresh();
	
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;

	void HandleOptionDecrease();
	void HandleOptionIncrease();
	void HandleRotatorChangedValue(int32 Value, bool bUserInitiated);

protected:
	UPROPERTY()
	TArray<FOptionEntryDiscreteValue> DiscreteValues;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonRotator* Rotator_OptionValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Button_Decrease;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Button_Increase;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_OptionName;
};

//////////////////////////////////////////////////////////////////////////
// UOptionListEntry_Toggler
//////////////////////////////////////////////////////////////////////////

DECLARE_MULTICAST_DELEGATE_OneParam(FOnToggleValueChangedDelegate, bool /*Value*/);

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API UOptionListEntry_Toggler : public UOptionListEntryBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitOption(const FText& InName, const bool InValue);

	void SetDisplayName(const FText& InName) override;

	UFUNCTION(BlueprintCallable)
	void SetToggleValue(const bool InValue);

	UFUNCTION(BlueprintPure)
	bool GetToggleValue();

	FOnToggleValueChangedDelegate OnToggleValueChangedDelegate;

protected:
	void NativeOnInitialized() override;

	UFUNCTION()
	void OnToggleValueChanged(bool Value);

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_OptionName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCheckBox* Cb_OptionValue;
};