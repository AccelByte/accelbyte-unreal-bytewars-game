// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemPurchaseSubsystem.h"
#include "OnlinePurchaseInterfaceAccelByte.h"
#include "StoreItemPurchaseLog.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"

void UStoreItemPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlinePurchasePtr PurchasePtr = Online::GetSubsystem(GetWorld())->GetPurchaseInterface();
	ensure(PurchasePtr);

	PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(PurchasePtr);
	ensure(PurchaseInterface);

	PurchaseInterface->OnCreateNewOrderCompleteDelegates.AddUObject(this, &ThisClass::OnCreateNewOrderComplete);
}

void UStoreItemPurchaseSubsystem::Deinitialize()
{
	Super::Deinitialize();

	PurchaseInterface->ClearOnCreateNewOrderCompleteDelegates(this);
}

void UStoreItemPurchaseSubsystem::CreateNewOrder(
	const APlayerController* OwningPlayer,
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const int32 SelectedPriceIndex,
	const int32 Quantity) const
{
	const UStoreItemPriceDataObject* SelectedPrice = StoreItemData->GetPrices()[SelectedPriceIndex];
	const FAccelByteModelsOrderCreate Order{
		StoreItemData->GetStoreItemId(),
		Quantity,
		static_cast<int32>(SelectedPrice->GetRegularPrice()) * Quantity,
		static_cast<int32>(SelectedPrice->GetFinalPrice()) * Quantity,
		FPreConfigCurrency::GetCodeFromType(SelectedPrice->GetCurrencyType())
	};

	PurchaseInterface->CreateNewOrder(
		*GetLocalPlayerUniqueNetId(OwningPlayer).Get(),
		Order);
}

void UStoreItemPurchaseSubsystem::OnCreateNewOrderComplete(
	bool bWasSuccessful,
	const FAccelByteModelsOrderInfo& OrderInfo,
	const FOnlineErrorAccelByte& OnlineError) const
{
	// Notify other object
	NotifyItemPurchased(OrderInfo);

	OnCheckoutCompleteDelegates.Broadcast(OnlineError);
}

void UStoreItemPurchaseSubsystem::NotifyItemPurchased(const FAccelByteModelsOrderInfo& OrderInfo) const
{
	// Get Item Data Asset
	const UTutorialModuleDataAsset* EntitlementModule =
		UTutorialModuleUtility::GetTutorialModuleDataAsset(FPrimaryAssetId("TutorialModule:ENTITLEMENTSESSENTIALS"), this);
	if (!EntitlementModule)
	{
		UE_LOG_STORE_ITEM_PURCHASE(Warning, "Entitlement essentials module is invalid. Make sure the module is active. Canceled.")
		return;
	}

	if (EntitlementModule->IsStarterModeActive())
	{
		UE_LOG_STORE_ITEM_PURCHASE(Warning, "Entitlement essentials have its starter mode activated. Canceled.")
		return;
	}

	UEntitlementsEssentialsSubsystem* EntitlementsSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem>();
	UInGameItemDataAsset* ItemDataAsset = EntitlementsSubsystem->GetItemDataAssetFromItemStoreId(OrderInfo.ItemId);
	if (!ItemDataAsset)
	{
		UE_LOG_STORE_ITEM_PURCHASE(Log, "Purchased item is not an in-game item. Canceled.")
		return;
	}

	// Get total owned from entitlement
	// Get first local player
	APlayerController* TargetPlayer = GetWorld()->GetFirstPlayerController();
	if (!TargetPlayer)
	{
		UE_LOG_STORE_ITEM_PURCHASE(Warning, "First player controller is invalid. Canceled.")
		return;
	}
	
	EntitlementsSubsystem->GetOrQueryUserItemEntitlement(
		TargetPlayer,
		OrderInfo.ItemId,
		FOnGetOrQueryUserItemEntitlementComplete::CreateWeakLambda(
			this,
			[OrderInfo, ItemDataAsset](const FOnlineError& Error, const UStoreItemDataObject* Entitlement)
			{
				if (!Error.bSucceeded)
				{
					UE_LOG_STORE_ITEM_PURCHASE(Warning, "Get entitlement failed with message: %s. Canceled.", *Error.ErrorMessage.ToString())
					return;
				}

				// Notify other objects
				if (OnItemPurchasedDelegates.IsBound()) 
				{
					OnItemPurchasedDelegates.Broadcast(
						OrderInfo.UserId,
						OrderInfo.OrderNo,
						ItemDataAsset->Id,
						OrderInfo.Quantity,
						Entitlement->GetCount() + OrderInfo.Quantity);
				}
			}));
}

#pragma region "Utilities"
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
#pragma endregion 
