// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "WalletEssentialsSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UWalletEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
#pragma region "Tutorial"
	// put your code here
#pragma endregion

private:
	FOnlineWalletAccelBytePtr WalletInterface;

#pragma region "Tutorial"
	// put your code here
#pragma endregion

#pragma region "Utilities"
	static int32 GetLocalUserNumFromPlayerController(const APlayerController* PlayerController);
#pragma endregion 
};
