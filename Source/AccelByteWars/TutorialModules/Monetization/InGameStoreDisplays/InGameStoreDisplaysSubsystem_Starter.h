// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "InGameStoreDisplaysModel.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Play/MatchSessionDS/MatchSessionDSOnlineSession_Starter.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "InGameStoreDisplaysSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UInGameStoreDisplaysSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
#pragma region "Tutorial"
	// Put your code here
#pragma endregion

private:
	FOnlineStoreV2AccelBytePtr StoreInterface;

#pragma region "Tutorial"
	// Put your code here
#pragma endregion

#pragma region "Utilities"
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const;
	UStoreItemDataObject* ConvertStoreData(const FOnlineStoreOfferAccelByteRef Offer) const;
#pragma endregion 
};
