// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "OwnedCountWidgetEntry.h"

#include "Components/TextBlock.h"
#include "TutorialModules/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"

void UOwnedCountWidgetEntry::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem>();
	ensure(EntitlementsSubsystem);

	SetVisibility(ESlateVisibility::Collapsed);
	if (EntitlementsSubsystem->GetIsQueryRunning())
	{
		Tb_OwnedCount->SetText(FText::FromString("..."));
	}
	else
	{
		ShowOwnedCount();
	}

	EntitlementsSubsystem->OnQueryUserEntitlementsCompleteDelegates.AddWeakLambda(
		this,
		[this](const FOnlineError& Error, const TArray<UItemDataObject*> Entitlements)
		{
			ShowOwnedCount();
		});
}

void UOwnedCountWidgetEntry::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	EntitlementsSubsystem->OnQueryUserEntitlementsCompleteDelegates.RemoveAll(this);
}

void UOwnedCountWidgetEntry::ShowOwnedCount()
{
	if (const UStoreItemListEntry* W_Parent = GetFirstOccurenceOuter<UStoreItemListEntry>())
	{
		const UItemDataObject* ItemData = W_Parent->GetItemData();
		if (!ItemData)
		{
			return;
		}

		const UItemDataObject* EntitlementData = EntitlementsSubsystem->GetItemEntitlement(GetOwningPlayer(), ItemData->Id);
		if (!EntitlementData)
		{
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("Amogus: %s %s: %d"), *GetOuter()->GetName(), *EntitlementData->Id, EntitlementData->Count);

		FText Text;
		if (EntitlementData->bConsumable && EntitlementData->Count > 0)
		{
			const FString TextString = FString::Printf(TEXT("%d x"), EntitlementData->Count);
			Text = FText::FromString(TextString);
		}
		else if (!EntitlementData->bConsumable)
		{
			Text = TEXT_OWNED;
		}
		Tb_OwnedCount->SetText(Text);
		SetVisibility(ESlateVisibility::Visible);
	}
}
