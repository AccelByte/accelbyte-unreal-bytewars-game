// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/SettingsListEntryBase.h"

#include "CommonTextBlock.h"
#include "AnalogSlider.h"
#include "CommonRotator.h"
#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"

void USettingsListEntryBase::NativeOnEntryReleased()
{
	// Cleanup operations
}

FReply USettingsListEntryBase::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	const UCommonInputSubsystem* InputSubsystem = GetInputSubsystem();
	if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		if (UWidget* PrimaryFocus = GetPrimaryGamepadFocusWidget())
		{
			TSharedPtr<SWidget> WidgetToFocus = PrimaryFocus->GetCachedWidget();
			if (WidgetToFocus.IsValid())
			{
				return FReply::Handled().SetUserFocus(WidgetToFocus.ToSharedRef(), InFocusEvent.GetCause());
			}
		}
	}

	return FReply::Unhandled();
}

void USettingsListEntryBase::SetDisplayName(const FText& InName)
{
	DisplayName = InName;
}

//////////////////////////////////////////////////////////////////////////
// USettingsListEntry_Scalar
//////////////////////////////////////////////////////////////////////////

void USettingsListEntry_Scalar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Slider_SettingValue->OnValueChanged.AddDynamic(this, &ThisClass::HandleSliderValueChanged);
}

void USettingsListEntry_Scalar::SetDisplayName(const FText& InName)
{
	Super::SetDisplayName(InName);
	
	OnDisplayNameChanged(DisplayName);
}

void USettingsListEntry_Scalar::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	ScalarSetting = 0.f;
}

void USettingsListEntry_Scalar::HandleSliderValueChanged(float Value)
{
	ScalarSetting = Value;

	Slider_SettingValue->SetValue(Value);
	Text_SettingValue->SetText(FText::FromString(FString::SanitizeFloat(Value)));

	OnValueChanged(Value);
}

void USettingsListEntry_Scalar::Refresh()
{
	Slider_SettingValue->SetValue(ScalarSetting);
	Slider_SettingValue->SetStepSize(0.1f);
	Text_SettingValue->SetText(FText::FromString(FString::SanitizeFloat(ScalarSetting)));

	Text_SettingName->SetText(DisplayName);
	OnDisplayNameChanged(DisplayName);

	TOptional<double> DefaultValue = ScalarSetting;
	OnDefaultValueChanged(DefaultValue.IsSet() ? DefaultValue.GetValue() : -1.0);

	OnValueChanged(ScalarSetting);

}

//////////////////////////////////////////////////////////////////////////
// USettingsListEntry_Discrete
//////////////////////////////////////////////////////////////////////////

void USettingsListEntry_Discrete::Refresh()
{
	TArray<FText> Options;
	for (FSettingsEntryDiscreteValue value : DiscreteValues)
	{
		Options.Emplace(value.DisplayName);
	}
	ensure(Options.Num() > 0);

	Text_SettingName->SetText(!DisplayName.IsEmpty() ? DisplayName : FText::FromString(TEXT("Settings Name Here")));
	
	Rotator_SettingValue->PopulateTextLabels(Options);
	Rotator_SettingValue->SetSelectedItem(0);
	//Rotator_SettingValue->SetDefaultOption(DiscreteSetting->GetDiscreteOptionDefaultIndex());
}

void USettingsListEntry_Discrete::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Rotator_SettingValue->OnRotatedEvent.AddUObject(this, &ThisClass::HandleRotatorChangedValue);
	Button_Decrease->OnClicked().AddUObject(this, &ThisClass::HandleOptionDecrease);
	Button_Increase->OnClicked().AddUObject(this, &ThisClass::HandleOptionIncrease);
}

void USettingsListEntry_Discrete::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	DiscreteValues.Empty();
}

void USettingsListEntry_Discrete::HandleOptionDecrease()
{
	Rotator_SettingValue->ShiftTextLeft();
	//DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
}

void USettingsListEntry_Discrete::HandleOptionIncrease()
{
	Rotator_SettingValue->ShiftTextRight();
	//DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
}

void USettingsListEntry_Discrete::HandleRotatorChangedValue(int32 Value, bool bUserInitiated)
{
	/*if (bUserInitiated)
	{
		DiscreteSetting->SetDiscreteOptionByIndex(Value);
	}*/
}