// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "EntitlementsEssentialsModel.h"
#include "EntitlementsEssentialsModel.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "EntitlementsEssentialsSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UEntitlementsEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
#pragma region Module Entitlement Essentials Function Declarations
	// TODO: Add your public function and variable declarations here.
#pragma endregion

private:
#pragma region Module Entitlement Essentials Function Declarations
	// TODO: Add your private function and variable declarations here.
#pragma endregion

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
	FOnlineEntitlementsAccelBytePtr EntitlementsInterface;

	UPROPERTY()
	TArray<UStoreItemDataObject*> StoreOffers;

	UPROPERTY()
	FPlayerEquipments CurrentEquipments;

#pragma region "Utilities"
	TArray<UStoreItemDataObject*> EntitlementsToDataObjects(TArray<TSharedRef<FOnlineEntitlement>> Entitlements) const;
	UStoreItemDataObject* EntitlementToDataObject(TSharedRef<FOnlineEntitlement> Entitlement) const;
#pragma endregion
};
