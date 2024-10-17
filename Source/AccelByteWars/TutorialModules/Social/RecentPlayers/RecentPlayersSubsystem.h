// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "Social/FriendsEssentials/FriendsEssentialsModels.h"
#include "RecentPlayersModels.h"
#include "RecentPlayersSubsystem.generated.h"

UCLASS()
class  ACCELBYTEWARS_API URecentPlayersSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

// @@@SNIPSTART RecentPlayersSubsystem.h-public
// @@@MULTISNIP GetRecentPlayersDeclaration {"selectedLines": ["1", "7-10"]}
// @@@MULTISNIP GetGameSessionPlayersDeclaration {"selectedLines": ["1", "12-13"]}
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController);
	
	void GetRecentPlayers(const APlayerController* PlayerController, const FOnGetRecentPlayersComplete& OnComplete = FOnGetRecentPlayersComplete());
	void QueryRecentPlayers(const APlayerController* PlayerController);
	FDelegateHandle BindRecentPlayerDelegate(FOnQueryRecentPlayersCompleteDelegate& Delegate);
	void UnBindRecentPlayerDelegate(FDelegateHandle& Delegate);

	void GetGameSessionPlayerList(const APlayerController* PlayerController, const FOnGetGameSessionPlayerListComplete& OnComplete = FOnGetGameSessionPlayerListComplete());
	FString GetGameSessionPlayerStatus(UFriendData* Player);
// @@@SNIPEND
	
// @@@SNIPSTART RecentPlayersSubsystem.h-protected
// @@@MULTISNIP Interface {"selectedLines": ["1-3"]}
protected:
	FOnlineFriendsAccelBytePtr FriendsInterface;
	IOnlineSessionPtr SessionInterface;
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.h-private
// @@@MULTISNIP UpdatePlayersInviteStatus {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnSessionDestroyed {"selectedLines": ["1", "3"]}
// @@@MULTISNIP RecentPlayersData {"selectedLines": ["1", "5-6"]}
// @@@MULTISNIP GameSessionPlayersData {"selectedLines": ["1", "8-9"]}
private:
	void UpdatePlayersInviteStatus(const APlayerController* PlayerController, const FOnGetGameSessionPlayerListComplete& OnComplete, TArray<UFriendData*>& PlayersData);
	void OnSessionDestroyed(FName SessionName, bool bWasSuccessful);

	UPROPERTY()
	TArray<UFriendData*> RecentPlayersData{};
	
	UPROPERTY()
	TArray<UFriendData*> GameSessionPlayersData{};
// @@@SNIPEND
};