// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "Social/FriendsEssentials/FriendsEssentialsModels.h"
#include "RecentPlayersModels.h"
#include "RecentPlayersSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API URecentPlayersSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController);

protected:
	FOnlineFriendsAccelBytePtr FriendsInterface;
	IOnlineSessionPtr SessionInterface;

#pragma region Module Recent Players Declarations
	// TODO: Add your Module Recent Players code here.
#pragma endregion
};
