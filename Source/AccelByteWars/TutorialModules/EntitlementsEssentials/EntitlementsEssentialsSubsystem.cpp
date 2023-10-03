// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "EntitlementsEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "TutorialModules/InGameStoreEssentials/UI/ShopWidget.h"
#include "TutorialModules/StoreItemPurchase/UI/ItemPurchaseWidget.h"

void UEntitlementsEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EntitlementsInterface = Online::GetSubsystem(GetWorld())->GetEntitlementsInterface();
	ensure(EntitlementsInterface);

	EntitlementsInterface->OnQueryEntitlementsCompleteDelegates.AddUObject(this, &ThisClass::OnQueryEntitlementComplete);

	UShopWidget::OnActivatedMulticastDelegate.AddUObject(this, &ThisClass::QueryUserEntitlement);
	UItemPurchaseWidget::OnPurchaseCompleteMulticastDelegate.AddUObject(this, &ThisClass::QueryUserEntitlement);
}

void UEntitlementsEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	EntitlementsInterface->OnQueryEntitlementsCompleteDelegates.RemoveAll(this);
	UShopWidget::OnActivatedMulticastDelegate.RemoveAll(this);
	UItemPurchaseWidget::OnPurchaseCompleteMulticastDelegate.RemoveAll(this);
}

UItemDataObject* UEntitlementsEssentialsSubsystem::GetItemEntitlement(
	const APlayerController* OwningPlayer,
	const FUniqueOfferId ItemId) const
{
	UItemDataObject* Item = nullptr;

	if (const TSharedPtr<FOnlineEntitlement> Entitlement = EntitlementsInterface->GetItemEntitlement(
		*GetLocalPlayerUniqueNetId(OwningPlayer).Get(),
		ItemId); Entitlement.IsValid())
	{
		Item = NewObject<UItemDataObject>();
		Item->Id = Entitlement->ItemId;
		Item->Title = FText::FromString(Entitlement->Name);
		Item->bConsumable = Entitlement->bIsConsumable;
		Item->Count = Entitlement->RemainingCount;
	}

	return Item;
}

void UEntitlementsEssentialsSubsystem::QueryUserEntitlement(const APlayerController* OwningPlayer)
{
	if (bIsQueryRunning)
	{
		return;
	}

	bIsQueryRunning = true;
	EntitlementsInterface->QueryEntitlements(
		*GetLocalPlayerUniqueNetId(OwningPlayer).Get(),
		"");
}

void UEntitlementsEssentialsSubsystem::OnQueryEntitlementComplete(
	bool bWasSuccessful,
	const FUniqueNetId& UserId,
	const FString& Namespace,
	const FString& Error)
{
	bIsQueryRunning = false;

	TArray<UItemDataObject*> Items;
	FOnlineError OnlineError;
	OnlineError.bSucceeded = bWasSuccessful;
	OnlineError.ErrorMessage = FText::FromString(Error);

	if (bWasSuccessful)
	{
		TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
		EntitlementsInterface->GetAllEntitlements(
			*GetLocalPlayerUniqueNetId(GetWorld()->GetFirstPlayerController()).Get(),
			"",
			Entitlements);

		for (const TSharedRef<FOnlineEntitlement>& Entitlement : Entitlements)
		{
			UItemDataObject* Item = NewObject<UItemDataObject>();
			Item->Id = Entitlement->ItemId;
			Item->Title = FText::FromString(Entitlement->Name);
			Item->bConsumable = Entitlement->bIsConsumable;
			Item->Count = Entitlement->RemainingCount;
			Items.Add(Item);
		}
	}
	
	OnQueryUserEntitlementsCompleteDelegates.Broadcast(OnlineError, Items);
}

FUniqueNetIdPtr UEntitlementsEssentialsSubsystem::GetLocalPlayerUniqueNetId(
	const APlayerController* PlayerController) const
{
	if (!ensure(PlayerController)) 
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
