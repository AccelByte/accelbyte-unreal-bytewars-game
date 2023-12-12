// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "PlayingWithFriendsSubsystem_Starter.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPlayingWithFriendsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Don't run on DS
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return IsRunningDedicatedServer() ? false : Super::ShouldCreateSubsystem(Outer);
	}

#pragma region "Helper"
public:
	bool IsInMatchSessionGameSession() const;

private:
	FUniqueNetIdRef GetSessionOwnerUniqueNetId(const FName SessionName) const;
	UPromptSubsystem* GetPromptSubsystem() const;

	void JoinGameSessionConfirmation(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& Invite);
	void OnQueryUserInfoOnGameSessionParticipantChange(
		const bool bSucceeded,
		const TArray<FUserOnlineAccountAccelByte*>& UsersInfo,
		FName SessionName,
		const bool bJoined);
#pragma endregion

#pragma region "Playing with Friends Function Declarations"
public:
	// TODO: Add your module public function declarations here.

private:
	// TODO: Add your module private function declarations here.
#pragma endregion

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	FUniqueNetIdPtr LeaderId;
	bool bLeaderChanged = false;
};
