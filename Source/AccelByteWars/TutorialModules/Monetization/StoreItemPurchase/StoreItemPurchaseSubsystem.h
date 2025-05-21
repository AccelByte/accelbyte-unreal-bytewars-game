// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineErrorAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "StoreItemPurchaseModel.h"
#include "OnlinePurchaseInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsModel.h"

#include "StoreItemPurchaseSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStoreItemPurchaseSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void CreateNewOrder(
		const APlayerController* OwningPlayer,
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
		const int32 SelectedPriceIndex,
		const int32 Quantity = 1) const;
	FOnOrderComplete OnCheckoutCompleteDelegates;

	static inline FOnItemPurchased OnItemPurchasedDelegates;

private:
	FOnlinePurchaseAccelBytePtr PurchaseInterface;

	void OnCreateNewOrderComplete(
		bool bWasSuccessful,
		const FAccelByteModelsOrderInfo& OrderInfo,
		const FOnlineErrorAccelByte& OnlineError) const;

	void NotifyItemPurchased(const FAccelByteModelsOrderInfo& OrderInfo) const;

#pragma region "Utilities"
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
#pragma endregion 
};
