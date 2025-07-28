// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineErrorAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "StoreItemPurchaseModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"

#include "StoreItemPurchaseSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStoreItemPurchaseSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

// @@@SNIPSTART StoreItemPurchaseSubsystem.h-public
// @@@MULTISNIP CreateNewOrder {"selectedLines": ["1-6"]}
// @@@MULTISNIP OnCheckoutCompleteDelegates {"selectedLines": ["1", "7"]}
public:
	void CreateNewOrder(
		const APlayerController* OwningPlayer,
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
		const int32 SelectedPriceIndex,
		const int32 Quantity = 1) const;
	FOnOrderComplete OnCheckoutCompleteDelegates;
// @@@SNIPEND

// @@@SNIPSTART StoreItemPurchaseSubsystem.h-private
// @@@MULTISNIP PurchaseInterface {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnCreateNewOrderComplete {"selectedLines": ["1", "4-7"]}
// @@@MULTISNIP GetLocalPlayerUniqueNetId {"selectedLines": ["1", "10"]}
private:
	FOnlinePurchaseAccelBytePtr PurchaseInterface;

	void OnCreateNewOrderComplete(
		bool bWasSuccessful,
		const FAccelByteModelsOrderInfo& OrderInfo,
		const FOnlineErrorAccelByte& OnlineError) const;

#pragma region "Utilities"
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
#pragma endregion
// @@@SNIPEND
};
