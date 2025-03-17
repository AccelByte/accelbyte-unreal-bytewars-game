// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/HelpOptions/Options/Components/OptionListEntryBase.h"

#include "CommonTextBlock.h"
#include "AnalogSlider.h"
#include "CommonRotator.h"
#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "Components/Image.h"
#include "Components/CheckBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"

void UOptionListEntryBase::NativeOnEntryReleased()
{

}

FReply UOptionListEntryBase::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
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

void UOptionListEntryBase::SetDisplayName(const FText& InName)
{
	DisplayName = InName;
}

//////////////////////////////////////////////////////////////////////////
// UOptionListEntry_Scalar
//////////////////////////////////////////////////////////////////////////

void UOptionListEntry_Scalar::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Slider_OptionValue->OnValueChanged.AddDynamic(this, &ThisClass::OnScalarValueChanged);
}

void UOptionListEntry_Scalar::InitOption(const FText& InName, const float InValue)
{
	SetDisplayName(InName);
	SetScalarValue(InValue);
}

void UOptionListEntry_Scalar::SetDisplayName(const FText& InName)
{
	Super::SetDisplayName(InName);
	Txt_OptionName->SetText(InName);
}

void UOptionListEntry_Scalar::SetScalarValue(const float InValue)
{
	Slider_OptionValue->SetValue(InValue);
	Img_ProgressBar->GetDynamicMaterial()->SetScalarParameterValue(TEXT("ScalarValue"), InValue);
	Txt_OptionValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), InValue)));
}

float UOptionListEntry_Scalar::GetScalarValue()
{
	return Slider_OptionValue->GetValue();
}

void UOptionListEntry_Scalar::OnScalarValueChanged(float Value)
{
	SetScalarValue(Value);
	OnScalarValueChangedDelegate.Broadcast(Value);
}

//////////////////////////////////////////////////////////////////////////
// UOptionListEntry_Discrete
//////////////////////////////////////////////////////////////////////////

void UOptionListEntry_Discrete::Refresh()
{
	TArray<FText> Options;
	for (FOptionEntryDiscreteValue value : DiscreteValues)
	{
		Options.Emplace(value.DisplayName);
	}
	ensure(Options.Num() > 0);

	Txt_OptionName->SetText(!DisplayName.IsEmpty() ? DisplayName : FText::FromString(TEXT("Settings Name Here")));
	
	Rotator_OptionValue->PopulateTextLabels(Options);
	Rotator_OptionValue->SetSelectedItem(0);
}

void UOptionListEntry_Discrete::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Rotator_OptionValue->OnRotatedEvent.AddUObject(this, &ThisClass::HandleRotatorChangedValue);
	Button_Decrease->OnClicked().AddUObject(this, &ThisClass::HandleOptionDecrease);
	Button_Increase->OnClicked().AddUObject(this, &ThisClass::HandleOptionIncrease);
}

void UOptionListEntry_Discrete::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	DiscreteValues.Empty();
}

void UOptionListEntry_Discrete::HandleOptionDecrease()
{
	Rotator_OptionValue->ShiftTextLeft();
}

void UOptionListEntry_Discrete::HandleOptionIncrease()
{
	Rotator_OptionValue->ShiftTextRight();
}

void UOptionListEntry_Discrete::HandleRotatorChangedValue(int32 Value, bool bUserInitiated)
{

}

//////////////////////////////////////////////////////////////////////////
// UOptionListEntry_Toggler
//////////////////////////////////////////////////////////////////////////

void UOptionListEntry_Toggler::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Cb_OptionValue->OnCheckStateChanged.AddDynamic(this, &UOptionListEntry_Toggler::OnToggleValueChanged);
}

void UOptionListEntry_Toggler::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Initialize widget
	SetDisplayName(DisplayName);
	SetCheckBoxSize(CheckBoxSize);
	SetTextStyle(TextStyle);
	SetTextToCheckBoxSeparation(TextToCheckBoxSeparation);
}

void UOptionListEntry_Toggler::InitOption(const FText& InName, const bool InValue)
{
	SetDisplayName(InName);
	SetToggleValue(InValue);
}

void UOptionListEntry_Toggler::SetDisplayName(const FText& InName)
{
	Super::SetDisplayName(InName);
	Txt_OptionName->SetText(InName);
}

void UOptionListEntry_Toggler::SetToggleValue(const bool InValue)
{
	Cb_OptionValue->SetCheckedState(InValue ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
}

void UOptionListEntry_Toggler::SetCheckBoxSize(const float InSize)
{
	Sb_OptionValueOuter->SetHeightOverride(InSize);
	Sb_OptionValueOuter->SetWidthOverride(InSize);
}

void UOptionListEntry_Toggler::SetTextStyle(const TSubclassOf<UCommonTextStyle> InTextStyle)
{
	Txt_OptionName->SetStyle(InTextStyle);
}

void UOptionListEntry_Toggler::SetTextToCheckBoxSeparation(const float InSize)
{
	S_Separation->SetSize(FVector2D(InSize, 1.0f));
}

bool UOptionListEntry_Toggler::GetToggleValue()
{
	return Cb_OptionValue->IsChecked();
}

void UOptionListEntry_Toggler::OnToggleValueChanged(bool Value)
{
	OnToggleValueChangedDelegate.Broadcast(Value);
}
