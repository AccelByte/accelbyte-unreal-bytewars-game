// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"

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

	for (UCommonButtonBase* Button : InputMethodDependantWidgets())
	{
		if (!Button)
		{
			continue;
		}

		Button->SetIsInteractionEnabled(bIsSelected);
	}
}

void UAccelByteWarsWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer())
	{
		InputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::ChangeInteractibility);
	}
}

void UAccelByteWarsWidgetEntry::ChangeInteractibility(ECommonInputType InputType)
{
	for (UCommonButtonBase* Button : InputMethodDependantWidgets())
	{
		if (!Button)
		{
			continue;
		}

		switch (InputType)
		{
		case ECommonInputType::Gamepad:
			Button->SetIsInteractionEnabled(IsListItemSelected());
			break;
		default:
			Button->SetIsInteractionEnabled(true);
		}
	}
}
