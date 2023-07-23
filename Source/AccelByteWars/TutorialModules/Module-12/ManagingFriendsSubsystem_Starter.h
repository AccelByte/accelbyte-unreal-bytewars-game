// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "TutorialModules/Module-8/FriendsEssentialsModels.h"
#include "TutorialModules/Module-12/ManagingFriendsLog.h"
#include "ManagingFriendsSubsystem_Starter.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UManagingFriendsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
#pragma region Module.12 Function Declarations
public:
	// TODO: Add your public Module.12 function declarations here.

protected:
	// TODO: Add your protected Module.12 function declarations here.

#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

protected:
	void OnUnfriendButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete = FOnUnfriendComplete());
	void OnBlockButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete = FOnBlockPlayerComplete());

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FDelegateHandle OnUnfriendCompleteDelegateHandle;
	FDelegateHandle OnQueryBlockedPlayersCompleteDelegateHandle;
	FDelegateHandle OnBlockPlayerCompleteDelegateHandle;
	FDelegateHandle OnUnblockPlayerCompleteDelegateHandle;
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;

	TMap<int32, FDelegateHandle> OnBlockedPlayersChangeDelegateHandles;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
};
