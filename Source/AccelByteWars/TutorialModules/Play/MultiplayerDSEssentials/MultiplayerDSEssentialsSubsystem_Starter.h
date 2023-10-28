// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerDSEssentialsSubsystemBase.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "MultiplayerDSEssentialsSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMultiplayerDSEssentialsSubsystem_Starter : public UMultiplayerDSEssentialsSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool IsAMSServer() const override { return false; }

private:
#pragma region "Tutorial module Multiplayer DS Essentials"
	// TODO: Declare your private functions here
#pragma endregion 

	bool bServerAlreadyRegister;
	bool bUnregisterServerRunning;
	FOnlineSessionV2AccelBytePtr ABSessionInt;
};
