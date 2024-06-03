// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "LoginQueueModel.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "LoginQueueSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API ULoginQueueSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
#pragma region "Tutorial"
	// place your code here
#pragma endregion

protected:
	FOnlineIdentityAccelBytePtr IdentityInterface;

#pragma region "Tutorial"
	// place your code here
#pragma endregion

#pragma region "Utilities"
private:
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PlayerController) const;
	APlayerController* GetPlayerControllerByLocalUserNum(const int32 LocalUserNum) const;
#pragma endregion 
};
