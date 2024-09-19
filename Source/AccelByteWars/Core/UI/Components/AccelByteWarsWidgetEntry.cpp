// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "CommonButtonBase.h"
#include "CommonInputSubsystem.h"

void UAccelByteWarsWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// set join button interactable or not (for gamepad navigation)
	ChangeInteractibility(InputSubsystem->GetCurrentInputType());
}

void UAccelByteWarsWidgetEntry::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);

	bIsItemSelected = bIsSelected;
	ChangeInteractibility(InputSubsystem->GetCurrentInputType());
}

void UAccelByteWarsWidgetEntry::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	// If button click sound is not assigned, use the default button sound
	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!ClickSound.GetResourceObject() && GameInstance)
	{
		ClickSound = GameInstance->GetDefaultButtonClickSound();
	}
	SetAllowClickSound(bAllowClickSound);
}

void UAccelByteWarsWidgetEntry::NativeConstruct()
{
	bAutoActivate = false;
	bOnActivatedInitializeFTUE = false;

	Super::NativeConstruct();

	InitializeGeneratedWidgets();

	if (const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer())
	{
		InputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::ChangeInteractibility);
	}
}

void UAccelByteWarsWidgetEntry::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	ChangeInteractibility(InputSubsystem->GetCurrentInputType());

	Super::NativeOnFocusLost(InFocusEvent);
}

FReply UAccelByteWarsWidgetEntry::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	ChangeInteractibility(InputSubsystem->GetCurrentInputType());

	return FReply::Handled();
}

FReply UAccelByteWarsWidgetEntry::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bAllowClickSound && ClickSound.GetResourceObject())
	{
		FSlateApplication::Get().PlaySound(ClickSound);
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UAccelByteWarsWidgetEntry::ChangeInteractibility(ECommonInputType InputType)
{
	for (UCommonButtonBase* Button : InputMethodDependantWidgets())
	{
		if (!Button)
		{
			continue;
		}
		
		bool bShouldInteractable = false;

		switch (InputType)
		{
		case ECommonInputType::Gamepad:
			bShouldInteractable = bIsItemSelected && HasUserFocus(GetOwningPlayer());
			break;
		default:
			bShouldInteractable = true;
		}

		Button->SetIsFocusable(bShouldInteractable);
		Button->SetIsInteractionEnabled(bShouldInteractable);
	}
}
