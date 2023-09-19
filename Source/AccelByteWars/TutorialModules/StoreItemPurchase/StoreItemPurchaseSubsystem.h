// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemUtils.h"
#include "StoreItemPurchaseModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "StoreItemPurchaseSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStoreItemPurchaseSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void Checkout(const APlayerController* OwningPlayer, const FUniqueOfferId OfferId, const int32 Quantity, const bool bIsConsumable);
	FOnCheckoutComplete OnCheckoutCompleteDelegates;

private:
	IOnlinePurchasePtr PurchaseInterface;

	void OnCheckoutComplete(const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt) const;
	
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
};
