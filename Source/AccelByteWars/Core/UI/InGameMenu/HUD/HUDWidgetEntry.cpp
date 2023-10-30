// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


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
