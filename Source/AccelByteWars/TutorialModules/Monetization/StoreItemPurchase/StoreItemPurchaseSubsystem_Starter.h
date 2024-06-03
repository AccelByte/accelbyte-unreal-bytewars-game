// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "StoreItemPurchaseModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"

#include "StoreItemPurchaseSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStoreItemPurchaseSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
#pragma region "Tutorial"
	// put your code here
#pragma endregion

private:
	FOnlinePurchaseAccelBytePtr PurchaseInterface;

	const TMap<ECurrencyType, FString> CurrencyCodeMap = {};

#pragma region "Tutorial"
	// put your code here
#pragma endregion 

#pragma region "Utilities"
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
#pragma endregion 
};
