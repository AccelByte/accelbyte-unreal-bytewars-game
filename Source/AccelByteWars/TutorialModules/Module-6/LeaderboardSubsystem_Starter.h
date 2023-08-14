// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineLeaderboardInterfaceAccelByte.h"
#include "TutorialModules/Module-6/LeaderboardEssentialsLog.h"
#include "TutorialModules/Module-6/LeaderboardEssentialsModels.h"
#include "LeaderboardSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API ULeaderboardSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
#pragma region Module.6 Function Declarations
public:
	// TODO: Add your public Module.6 function declarations here.
protected:
	// TODO: Add your protected Module.6 function declarations here.
#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineLeaderboardAccelBytePtr LeaderboardInterface;

	FDelegateHandle OnLeaderboardReadCompleteDelegateHandle;
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
};
