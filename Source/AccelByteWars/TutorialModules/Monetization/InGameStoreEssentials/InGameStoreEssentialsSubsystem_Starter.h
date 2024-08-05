// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "InGameStoreEssentialsModel.h"
#include "OnlineSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "InGameStoreEssentialsSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UInGameStoreEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
#pragma region "Tutorial"
	// put your code here
#pragma endregion 

private:
	IOnlineStoreV2Ptr StoreInterface;

#pragma region "Tutorial"
	// put your code here
#pragma endregion 

#pragma region "Utilities"
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const;
	UStoreItemDataObject* ConvertStoreData(
		const FOnlineStoreOffer& Offer) const;
#pragma endregion 
};
