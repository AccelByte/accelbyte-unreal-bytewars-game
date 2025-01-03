// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "LoginQueueSubsystem_Starter.h"

#include "LoginQueueLog.h"
#include "OnlineSubsystemUtils.h"

void ULoginQueueSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Get Online Subsystem and make sure it's valid.
	FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	if (!ensure(Subsystem)) 
	{
		UE_LOG_LOGIN_QUEUE(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return;
	}

	// Grab the reference of AccelByte Identity Interface and make sure it's valid.
	IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		UE_LOG_LOGIN_QUEUE(Warning, TEXT("Identity interface is not valid."));
		return;
	}

#pragma region "Tutorial"
	// place your code here
#pragma endregion
}

void ULoginQueueSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

#pragma region "Tutorial"
	// place your code here
#pragma endregion
}

#pragma region "Tutorial"
// place your code here
#pragma endregion

#pragma region "Utilities"
int32 ULoginQueueSubsystem_Starter::GetLocalUserNumFromPlayerController(const APlayerController* PlayerController) const
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

APlayerController* ULoginQueueSubsystem_Starter::GetPlayerControllerByLocalUserNum(const int32 LocalUserNum) const
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
#pragma endregion 
