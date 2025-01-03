// Copyright (c) 2024 Native AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "NativeFriendsEssentialsLog.h"
#include "NativeFriendsEssentialsModels.h"
#include "NativeFriendsSubsystem_Starter.generated.h"

class UFriendsSubsystem_Starter;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UNativeFriendsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

#pragma region Module Read Function Declarations
public:
	// TODO: Add your public Module Get Native Friend List function declarations here.

protected:
	// TODO: Add your protected Module Get Native Friend List function declarations here.
#pragma endregion

#pragma region Module Sync Native Friend List Function Declarations
public:
	// TODO: Add your public Module Sync Native Friend List function declarations here.

protected:
	// TODO: Add your protected Sync Native Friend List function declarations here.
#pragma endregion

protected:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FOnlineSubsystemAccelByte* ABSubsystem;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
	FOnlineUserCacheAccelBytePtr UserCache;
};
