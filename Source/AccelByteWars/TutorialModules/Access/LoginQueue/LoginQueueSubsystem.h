// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "LoginQueueModel.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "LoginQueueSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API ULoginQueueSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void CancelLoginQueue(const APlayerController* PlayerController) const;

	FOnLoginQueueCancelCompleted OnLoginQueueCancelCompletedDelegates;
	FOnLoginQueued OnLoginQueuedDelegates;
	FOnLoginTicketStatusUpdated OnLoginTicketStatusUpdatedDelegates;

protected:
	FOnlineIdentityAccelBytePtr IdentityInterface;

	void OnLoginQueueCancelCompleted(const int32 PlayerNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error) const;
	void OnLoginQueued(const int32 PlayerNum, const FAccelByteModelsLoginQueueTicketInfo& TicketInfo) const;
	void OnLoginTicketStatusUpdated(
		const int32 PlayerNum,
		bool bWasSuccessful,
		const FAccelByteModelsLoginQueueTicketInfo& TicketInfo,
		const FOnlineErrorAccelByte& Error) const;

#pragma region "Utilities"
private:
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PlayerController) const;
	APlayerController* GetPlayerControllerByLocalUserNum(const int32 LocalUserNum) const;
#pragma endregion 
};
