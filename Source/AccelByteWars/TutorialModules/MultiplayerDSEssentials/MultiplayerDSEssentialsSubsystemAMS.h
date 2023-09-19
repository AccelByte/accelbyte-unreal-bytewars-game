// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerDSEssentialsSubsystemBase.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "MultiplayerDSEssentialsSubsystemAMS.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMultiplayerDSEssentialsSubsystemAMS : public UMultiplayerDSEssentialsSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool IsAMSServer() const override { return true; }

private:
	void RegisterServer(const FName SessionName);
	void OnRegisterServerComplete(const bool bSucceeded);

	void UnregisterServer(const FName SessionName);
	void OnUnregisterServerComplete(const bool bSucceeded);

	bool bServerAlreadyRegister;
	bool bUnregisterServerRunning;
	FOnlineSessionV2AccelBytePtr ABSessionInt;
};
