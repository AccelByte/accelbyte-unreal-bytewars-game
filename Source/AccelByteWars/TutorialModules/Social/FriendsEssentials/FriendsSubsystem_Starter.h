// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "FriendsEssentialsLog.h"
#include "FriendsEssentialsModels.h"
#include "FriendsSubsystem_Starter.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UFriendsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

#pragma region Module.8a Function Declarations
public:
	// TODO: Add your public Module.8a function declarations here.

protected:
	// TODO: Add your protected Module.8a function declarations here.
#pragma endregion

#pragma region Module.8b Function Declarations
public:
	// TODO: Add your public Module.8b function declarations here.

protected:
	// TODO: Add your protected Module.8b function declarations here.
#pragma endregion

#pragma region Module.8c Function Declarations
public:
	// TODO: Add your public Module.8c function declarations here.
#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
	FDelegateHandle OnRejectFriendRequestCompleteDelegateHandle;
	FDelegateHandle OnCancelFriendRequestCompleteDelegateHandle;

	TMap<int32, FDelegateHandle> OnFriendsChangeDelegateHandles;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
};