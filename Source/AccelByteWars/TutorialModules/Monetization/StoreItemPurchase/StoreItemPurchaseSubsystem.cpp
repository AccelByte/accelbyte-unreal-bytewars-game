// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemPurchaseSubsystem.h"

void UStoreItemPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PurchaseInterface = Online::GetSubsystem(GetWorld())->GetPurchaseInterface();
	ensure(PurchaseInterface);
}

void UStoreItemPurchaseSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UStoreItemPurchaseSubsystem::Checkout(
	const APlayerController* OwningPlayer,
	const FUniqueOfferId OfferId,
	const int32 Quantity,
	const bool bIsConsumable)
{
	FPurchaseCheckoutRequest Request;
	FPurchaseCheckoutRequest::FPurchaseOfferEntry PurchaseOfferEntry{
		FOfferNamespace(),
		OfferId,
		Quantity,
		bIsConsumable
	};
	Request.PurchaseOffers.Add(PurchaseOfferEntry);

	PurchaseInterface->Checkout(
		*GetLocalPlayerUniqueNetId(OwningPlayer).Get(),
		Request,
		FOnPurchaseCheckoutComplete::CreateUObject(this, &ThisClass::OnCheckoutComplete));
}

void UStoreItemPurchaseSubsystem::OnCheckoutComplete(
	const FOnlineError& Result,
	const TSharedRef<FPurchaseReceipt>& Receipt) const
{
	OnCheckoutCompleteDelegates.Broadcast(Result);
}

FUniqueNetIdPtr UStoreItemPurchaseSubsystem::GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const
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
