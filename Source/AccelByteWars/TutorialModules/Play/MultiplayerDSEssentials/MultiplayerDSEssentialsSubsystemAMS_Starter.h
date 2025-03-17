// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerDSEssentialsSubsystemBase.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "MultiplayerDSEssentialsSubsystemAMS_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMultiplayerDSEssentialsSubsystemAMS_Starter : public UMultiplayerDSEssentialsSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool IsAMSServer() const override { return true; }

private:
#pragma region "Tutorial module Multiplayer DS Essentials AMS"
	// TODO: Declare your private functions here
#pragma endregion 

	TSharedPtr<FAccelByteModelsV2GameSession> GetSessionInfo(const FName SessionName) const;

	bool bServerAlreadyRegister;
	bool bUnregisterServerRunning;
	FOnlineSessionV2AccelBytePtr ABSessionInt;
};
