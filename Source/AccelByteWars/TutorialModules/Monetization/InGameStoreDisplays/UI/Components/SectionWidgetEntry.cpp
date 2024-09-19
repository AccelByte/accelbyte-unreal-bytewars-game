// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "SectionWidgetEntry.h"

#include "Components/Border.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Monetization/InGameStoreDisplays/InGameStoreDisplaysModel.h"
#include "Monetization/InGameStoreEssentials/UI/StoreItemDetailWidget.h"

void USectionWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const USectionDataObject* SectionDataObject = Cast<USectionDataObject>(ListItemObject);
	if (!SectionDataObject)
	{
		return;
	}

	TimeLeft = SectionDataObject->SectionInfo.EndDate - FDateTime::Now();
	B_SectionBorder->SetBrushColor(SectionDataObject->SectionColor);
	Tb_SectionName->SetText(FText::FromString(SectionDataObject->SectionInfo.Title));
	Lv_Items->OnItemClicked().AddUObject(this, &ThisClass::OnItemClicked);
	Lv_Items->SetListItems(SectionDataObject->Offers);
}

void USectionWidgetEntry::NativeOnEntryReleased()
{
	Lv_Items->OnItemClicked().Clear();
	IUserObjectListEntry::NativeOnEntryReleased();
}

void USectionWidgetEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TimeLeft -= InDeltaTime;

	if (TimeLeft > 0)
	{
		const FString TimeLeftString = TimeLeft.ToString(TEXT("%d d %h h"));
		Tb_TimeLeft->SetText(FText::FromString(TimeLeftString));
	}
}

void USectionWidgetEntry::OnItemClicked(UObject* Item) const
{
	if (!IsValid(DetailWidgetClass))
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	UAccelByteWarsBaseUI* BaseUI = GameInstance->GetBaseUIWidget();
	if (!BaseUI)
	{
		return;
	}

	UAccelByteWarsActivatableWidget* Widget = BaseUI->PushWidgetToStack(EBaseUIStackType::Menu, DetailWidgetClass);

	UStoreItemDetailWidget* ItemDetailWidget = Cast<UStoreItemDetailWidget>(Widget);
	const UStoreItemDataObject* StoreItem = Cast<UStoreItemDataObject>(Item);
	if (!ItemDetailWidget || !StoreItem)
	{
		return;
	}

	ItemDetailWidget->Setup(StoreItem);
}
