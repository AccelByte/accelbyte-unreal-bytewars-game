// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PlatformIconWidgetEntry.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Access/AuthEssentials/AuthEssentialsModels.h"
#include "Components/Image.h"

void UPlatformIconWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UPlatformWidgetData* PlatformWidgetData = Cast<UPlatformWidgetData>(ListItemObject);
	if (!PlatformWidgetData)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const EAccelBytePlatformType PlatformType = UAuthEssentialsModels::GetPlatformType(PlatformWidgetData->GetNetId());
	if (const FSlateBrush* Brush = BrushMap.Find(PlatformType))
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Img_Icon->SetBrush(*Brush);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}
