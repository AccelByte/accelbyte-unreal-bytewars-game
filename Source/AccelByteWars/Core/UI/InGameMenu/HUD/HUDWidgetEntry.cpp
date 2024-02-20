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

void UHUDWidgetEntry::SetPowerUpValues(const TArray<TEnumAsByte<EPowerUpSelection>>& SelectedPowerUps, const TArray<int32>& PowerUpCounts)
{
	// Hide the power ups first.
	for (auto& PowerUpWidget : PowerUpWidgets) 
	{
		PowerUpWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Show the power ups if valid.
	int32 PowerUpIndex = 0;
	for (const auto& PowerUp : SelectedPowerUps)
	{
		if (PowerUpWidgets.IsValidIndex(PowerUpIndex) && PowerUpCounts.IsValidIndex(PowerUpIndex))
		{
			const int32 PowerUpCount = PowerUpCounts[PowerUpIndex];
			if (PowerUp != 0 && PowerUpCount > 0)
			{
				PowerUpWidgets[PowerUpIndex]->SetValue(PowerUp, PowerUpCount);
				PowerUpWidgets[PowerUpIndex]->SetVisibility(ESlateVisibility::Visible);
			}
		}

		PowerUpIndex++;
	}
}
