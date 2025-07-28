// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "EntitlementsEssentialsModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsModel.h"
#include "EntitlementsEssentialsSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UEntitlementsEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
#pragma region "Tutorial"
	// Put your code here.
#pragma endregion 

private:
	FOnlineEntitlementsAccelBytePtr EntitlementsInterface;

	UPROPERTY()
	TArray<UStoreItemDataObject*> StoreOffers;

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion

#pragma region "Utilities"
	TArray<UStoreItemDataObject*> EntitlementsToDataObjects(TArray<TSharedRef<FOnlineEntitlement>> Entitlements) const;
	UStoreItemDataObject* EntitlementToDataObject(TSharedRef<FOnlineEntitlement> Entitlement) const;
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
#pragma endregion
};
