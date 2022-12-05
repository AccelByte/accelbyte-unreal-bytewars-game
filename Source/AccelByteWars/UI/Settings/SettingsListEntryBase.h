// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserListEntry.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "SettingsListEntryBase.generated.h"

class UAnalogSlider;
class USlider;
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
///
UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API USettingsListEntry_Scalar : public USettingsListEntryBase
{
	GENERATED_BODY()

public:
	virtual void SetDisplayName(const FText& InName) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	float ScalarSetting;
	
protected:
	UFUNCTION(BlueprintCallable)
	void Refresh();
	
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;

	UFUNCTION()
	void HandleSliderValueChanged(float Value);

	UFUNCTION(BlueprintImplementableEvent)
	void OnValueChanged(float Value);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDefaultValueChanged(float DefaultValue);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDisplayNameChanged(const FText& InName);

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAnalogSlider* Slider_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Text_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonTextBlock* Text_SettingName;
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
	UCommonTextBlock* Text_SettingName;
};
