// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/InGameMenu/HUD/HUDWidgetEntry.h"
#include "Core/UI/InGameMenu/HUD/PowerUpWidgetEntry.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UHUDWidgetEntry::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetColorAndOpacity(Color);

	Text_Label_Left->SetText(LabelLeftText);
	Text_Label_Middle->SetText(LabelMiddleText);
	Text_Label_Right->SetText(LabelRightText);

	const ESlateVisibility TargetVisibility = bHideLeftRight ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	Text_Label_Left->SetVisibility(TargetVisibility);
	Text_Value_Left->SetVisibility(TargetVisibility);
	Text_Label_Right->SetVisibility(TargetVisibility);
	Text_Value_Right->SetVisibility(TargetVisibility);

	PowerUpWidgets.Empty();
	PowerUpWidgets.Add(Widget_PowerUpP1);
	PowerUpWidgets.Add(Widget_PowerUpP2);
	PowerUpWidgets.Add(Widget_PowerUpP3);
	PowerUpWidgets.Add(Widget_PowerUpP4);

	Hb_PowerUps->SetVisibility(bHidePowerUpWidgets ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UHUDWidgetEntry::SetPowerUpValues(const FString& ItemId, const int32 Count, const int32 MemberIndex)
{
	if (!PowerUpWidgets.IsValidIndex(MemberIndex))
	{
		return;
	}
	UPowerUpWidgetEntry* TargetWidget = PowerUpWidgets[MemberIndex];

	// Show the power ups if valid.
	if (!ItemId.IsEmpty())
	{
		TargetWidget->SetValue(ItemId, Count);
		TargetWidget->SetVisibility(Count > 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
