// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsButtonBase.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/GameUIManagerSubsystem.h"
#include "CommonActionWidget.h"

void UAccelByteWarsButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	// If button click sound is not assigned, use the default button sound
	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ClickSound = PressedSlateSoundOverride;
	if (!ClickSound.GetResourceObject() && GameInstance)
	{
		ClickSound = GameInstance->GetDefaultButtonClickSound();
	}

	SetAllowClickSound(bAllowClickSound);

	UpdateButtonStyle();
	RefreshButtonText();
	RefreshButtonImage();

	SetButtonType(ButtonType);
	ToggleExclamationMark(bShowExclamationMark);
}

void UAccelByteWarsButtonBase::NativeConstruct()
{
	Super::NativeConstruct();

	SetButtonType(ButtonType);
	ToggleExclamationMark(bShowExclamationMark);

	OnClicked().RemoveAll(this);
	OnClicked().AddUObject(this, &ThisClass::ToggleExclamationMark, false);
}

void UAccelByteWarsButtonBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	if (UGameUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UGameUIManagerSubsystem>())
	{
		if (UAccelByteWarsButtonBase* LastHoveredButton = UIManager->GetLastHoveredButton())
		{
			if (LastHoveredButton->IsHovered())
			{
				return;
			}
		}

		UIManager->SetSelectedButton(this);
		SetSelectedInternal(true, false);
		bSelectedByKeyboard = true;
	}
}

void UAccelByteWarsButtonBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	SetSelectedInternal(false, false);
	bSelectedByKeyboard = false;
}

void UAccelByteWarsButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	UpdateButtonStyle();
	RefreshButtonText();
	RefreshButtonImage();
}

void UAccelByteWarsButtonBase::SetButtonText(const FText& InText)
{
	bOverride_ButtonText = InText.IsEmpty();
	ButtonText = InText;
	RefreshButtonText();
}

void UAccelByteWarsButtonBase::SetButtonImage(const FSlateBrush& InBrush)
{
	ButtonBrush = InBrush;
	RefreshButtonImage();
}

void UAccelByteWarsButtonBase::SetAllowClickSound(bool bAllow)
{
	bAllowClickSound = bAllow;
	SetPressedSoundOverride(bAllowClickSound ? Cast<USoundBase>(ClickSound.GetResourceObject()) : nullptr);
}

void UAccelByteWarsButtonBase::RefreshButtonText()
{
	if (!bOverride_ButtonText || ButtonText.IsEmpty())
	{
		if (InputActionWidget)
		{
			const FText ActionDisplayText = InputActionWidget->GetDisplayText();
			if (!ActionDisplayText.IsEmpty())
			{
				UpdateButtonText(ActionDisplayText);
				return;
			}
		}
	}

	UpdateButtonText(ButtonText);
}

void UAccelByteWarsButtonBase::RefreshButtonImage()
{
	UpdateButtonImage(ButtonBrush);
}

void UAccelByteWarsButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);

	UpdateButtonStyle();
}

void UAccelByteWarsButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();

	if (UGameUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UGameUIManagerSubsystem>())
	{
		UIManager->SetHoveredButton(this);

		if (UAccelByteWarsButtonBase* LastSelectedButton = UIManager->GetLastSelectedButton())
		{
			LastSelectedButton->SetSelectedInternal(false, false);
		}
	}
}

void UAccelByteWarsButtonBase::ClearButtonBindings()
{
	OnClicked().Clear();
}

void UAccelByteWarsButtonBase::CallOnClick()
{
	if (OnClicked().IsBound())
	{
		OnClicked().Broadcast();
	}
}