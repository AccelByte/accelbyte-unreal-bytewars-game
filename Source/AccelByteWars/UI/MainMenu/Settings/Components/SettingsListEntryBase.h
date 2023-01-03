// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserListEntry.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "SettingsListEntryBase.generated.h"

class UAnalogSlider;
class USlider;
class UImage;
class UCommonTextBlock;
class UCommonButtonBase;
class UCommonRotator;

// Settings Entry Type
UENUM(BlueprintType)
enum class ESettingsEntryType : uint8
{
	EntryScalar,
	EntryDiscrete
};

USTRUCT(BlueprintType)
struct FSettingsEntryDiscreteValue
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

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable)
class ACCELBYTEWARS_API USettingsListEntryBase : public UCommonUserWidget, public IUserObjectListEntry
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

	ESettingsEntryType EntryType;
};

//////////////////////////////////////////////////////////////////////////
// USettingsListEntry_Scalar
//////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScalarValueChangedDelegate, float, Value);

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API USettingsListEntry_Scalar : public USettingsListEntryBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitSetting(const FText& InName, const float InValue);

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
	UAnalogSlider* Slider_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_SettingName;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Discrete
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API USettingsListEntry_Discrete : public USettingsListEntryBase
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
	// 
	// UPROPERTY()
	// UGameSettingValueDiscrete* DiscreteSetting;

	UPROPERTY()
	TArray<FSettingsEntryDiscreteValue> DiscreteValues;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonRotator* Rotator_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Button_Decrease;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Button_Increase;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Txt_SettingName;
};
