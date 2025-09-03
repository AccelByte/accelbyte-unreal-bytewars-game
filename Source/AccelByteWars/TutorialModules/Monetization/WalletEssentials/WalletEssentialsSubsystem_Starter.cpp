// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletEssentialsSubsystem_Starter.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineWalletV2InterfaceAccelByte.h"

void UWalletEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	ensure(Subsystem);

	WalletInterface = Subsystem->GetWalletV2Interface();
	ensure(WalletInterface);

	// put your code here
}

void UWalletEssentialsSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// put your code here
}

#pragma region "Tutorial"
// put your code here
#pragma endregion 

#pragma region "Utilities"
int32 UWalletEssentialsSubsystem_Starter::GetLocalUserNumFromPlayerController(const APlayerController* PlayerController)
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
#pragma endregion 
