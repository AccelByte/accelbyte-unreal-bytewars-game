// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"
#include "CustomMatchSubsystem.generated.h"

class UAccelByteWarsOnlineSessionBase;

UCLASS()
class ACCELBYTEWARS_API UCustomMatchSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void CreateCustomGameSession(
		const int32 LocalUserNum,
		const EGameModeNetworkType NetworkType,
		const EGameModeType GameModeType,
		const EGameStyle GameStyle,
		const EAccelByteV2SessionJoinability Joinability,
		const int32 Duration,
		const int32 PlayerLives,
		const int32 MissileLimit /* -1 = unlimited*/,
		const int32 MaxPlayerTotalNum,
		const int32 MaxTeamNum);
	FOnCreateSessionComplete OnCreateCustomGameSessionCompleteDelegates;

	void LeaveGameSession() const;
	FOnDestroySessionComplete OnLeaveGameSessionDelegates;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	const FString FallbackSessionTemplate = TEXT("unreal-elimination-ds-ams");

	void OnCreateSessionComplete(const FName SessionName, const bool bSucceeded) const;
	void OnLeaveGameSessionComplete(const FName SessionName, const bool bSucceeded) const;
	void OnSessionServerUpdateReceived(
		const FName SessionName,
		const FOnlineError& Error,
		const bool bHasClientTravelTriggered) const;
};
