// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreEssentialsSubsystem_Starter.h"

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"

void UInGameStoreEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	StoreInterface = Subsystem->GetStoreV2Interface();
	ensure(StoreInterface);
}

#pragma region "Tutorial"
// Put your code here.
#pragma endregion 

#pragma region "Utilities"
FUniqueNetIdPtr UInGameStoreEssentialsSubsystem_Starter::GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	const int LocalUserNum = LocalPlayer->GetControllerId();

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	return Subsystem->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
}
#pragma endregion 
