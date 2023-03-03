// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/UI/InGameMenu/HUD/HUDWidgetEntry.h"

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
}
