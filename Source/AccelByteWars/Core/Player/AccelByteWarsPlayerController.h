// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerController.h"
#include "AccelByteWarsPlayerController.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Trigger to start the game from game lobby.
	UFUNCTION(Reliable, Server, meta = (WorldContext = "WorldContextObject"))
	void TriggerLobbyStart();

	void LoadingPlayerAssignment() const;

	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());
};