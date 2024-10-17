// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "LoginQueueSubsystem.h"

#include "LoginQueueLog.h"
#include "OnlineSubsystemUtils.h"

// @@@SNIPSTART LoginQueueSubsystem.cpp-Initialize
// @@@MULTISNIP Interface {"selectedLines": ["1-19", "24"]}
// @@@MULTISNIP LoginQueueBinding {"selectedLines": ["1-2", "22-24"]}
// @@@MULTISNIP CancelQueueBinding {"selectedLines": ["1-2", "21", "24"]}
void ULoginQueueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Get Online Subsystem and make sure it's valid.
	FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	if (!ensure(Subsystem)) 
	{
		UE_LOG_LOGIN_QUEUE(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
		return;
	}

	// Grab the reference of AccelByte Identity Interface and make sure it's valid.
	IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		UE_LOG_LOGIN_QUEUE(Warning, TEXT("Identiy interface is not valid."));
		return;
	}

	IdentityInterface->AccelByteOnLoginQueueCancelCompleteDelegates->AddUObject(this, &ThisClass::OnLoginQueueCancelCompleted);
	IdentityInterface->AccelByteOnLoginQueuedDelegates->AddUObject(this, &ThisClass::OnLoginQueued);
	IdentityInterface->AccelByteOnLoginTicketStatusUpdatedDelegates->AddUObject(this, &ThisClass::OnLoginTicketStatusUpdated);
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueSubsystem.cpp-Deinitialize
// @@@MULTISNIP LoginQueueUnbinding {"selectedLines": ["1-2", "6-8"]}
// @@@MULTISNIP CancelQueueUnbinding {"selectedLines": ["1-2", "5", "8"]}
void ULoginQueueSubsystem::Deinitialize()
{
	Super::Deinitialize();

	IdentityInterface->AccelByteOnLoginQueueCancelCompleteDelegates->RemoveAll(this);
	IdentityInterface->AccelByteOnLoginQueuedDelegates->RemoveAll(this);
	IdentityInterface->AccelByteOnLoginTicketStatusUpdatedDelegates->RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueSubsystem.cpp-CancelLoginQueue
void ULoginQueueSubsystem::CancelLoginQueue(const APlayerController* PlayerController) const
{
	IdentityInterface->CancelLoginQueue(GetLocalUserNumFromPlayerController(PlayerController));
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueSubsystem.cpp-OnLoginQueueCancelCompleted
void ULoginQueueSubsystem::OnLoginQueueCancelCompleted(
	const int32 PlayerNum,
	bool bWasSuccessful,
	const FOnlineErrorAccelByte& Error) const
{
	// OnLoginComplete will also be triggered after this is triggered

	FOnlineError OutError = Error;
	OutError.bSucceeded = bWasSuccessful;

	OnLoginQueueCancelCompletedDelegates.Broadcast(GetPlayerControllerByLocalUserNum(PlayerNum), OutError);
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueSubsystem.cpp-OnLoginQueued
void ULoginQueueSubsystem::OnLoginQueued(const int32 PlayerNum, const FAccelByteModelsLoginQueueTicketInfo& TicketInfo) const
{
	OnLoginQueuedDelegates.Broadcast(GetPlayerControllerByLocalUserNum(PlayerNum), TicketInfo);
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueSubsystem.cpp-OnLoginTicketStatusUpdated
void ULoginQueueSubsystem::OnLoginTicketStatusUpdated(
	const int32 PlayerNum,
	bool bWasSuccessful,
	const FAccelByteModelsLoginQueueTicketInfo& TicketInfo,
	const FOnlineErrorAccelByte& Error) const
{
	// bWasSuccessful false means the player still in queue, not the request has failed
	OnLoginTicketStatusUpdatedDelegates.Broadcast(GetPlayerControllerByLocalUserNum(PlayerNum), TicketInfo, Error);
}
// @@@SNIPEND

#pragma region "Utilities"
// @@@SNIPSTART LoginQueueSubsystem.cpp-GetLocalUserNumFromPlayerController
int32 ULoginQueueSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return INDEX_NONE;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return INDEX_NONE;
	}

	return LocalPlayer->GetControllerId();
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueSubsystem.cpp-GetPlayerControllerByLocalUserNum
APlayerController* ULoginQueueSubsystem::GetPlayerControllerByLocalUserNum(const int32 LocalUserNum) const
{
	APlayerController* MatchedPC = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			continue;
		}

		if (APlayerController* PC = It->Get(); PC)
		{
			if (LocalUserNum == GetLocalUserNumFromPlayerController(PC))
			{
				MatchedPC = PC;
				break;
			}
		}
	}
	return MatchedPC;
}
// @@@SNIPEND
#pragma endregion 
