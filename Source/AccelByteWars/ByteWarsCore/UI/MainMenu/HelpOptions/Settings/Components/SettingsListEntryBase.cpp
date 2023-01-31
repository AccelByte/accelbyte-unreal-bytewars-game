// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/UI/MainMenu/HelpOptions/Settings/Components/SettingsListEntryBase.h"

#include "CommonTextBlock.h"
#include "AnalogSlider.h"
#include "CommonRotator.h"
#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "Components/Image.h"

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
	Slider_SettingValue->OnValueChanged.AddDynamic(this, &ThisClass::OnScalarValueChanged);
}

void USettingsListEntry_Scalar::InitSetting(const FText& InName, const float InValue)
{
	SetDisplayName(InName);
	SetScalarValue(InValue);
}

void USettingsListEntry_Scalar::SetDisplayName(const FText& InName)
{
	Super::SetDisplayName(InName);
	Txt_SettingName->SetText(InName);
}

void USettingsListEntry_Scalar::SetScalarValue(const float InValue)
{
	Slider_SettingValue->SetValue(InValue);
	Img_ProgressBar->GetDynamicMaterial()->SetScalarParameterValue(TEXT("ScalarValue"), InValue);
	Txt_SettingValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), InValue)));
}

float USettingsListEntry_Scalar::GetScalarValue()
{
	return Slider_SettingValue->GetValue();
}

void USettingsListEntry_Scalar::OnScalarValueChanged(float Value)
{
	SetScalarValue(Value);
	OnScalarValueChangedDelegate.Broadcast(Value);
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

	Txt_SettingName->SetText(!DisplayName.IsEmpty() ? DisplayName : FText::FromString(TEXT("Settings Name Here")));
	
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