// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"

#include "Net/UnrealNetwork.h"

void AAccelByteWarsMainMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, LobbyStatus);
	DOREPLIFETIME(ThisClass, LobbyCountdown);
}
