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

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.h-private
// @@@MULTISNIP RegisterServer {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnRegisterServerComplete {"selectedLines": ["1", "3"]}
// @@@MULTISNIP UnregisterServer {"selectedLines": ["1", "5"]}
// @@@MULTISNIP OnUnregisterServerComplete {"selectedLines": ["1", "6"]}
// @@@MULTISNIP SendServerReady {"selectedLines": ["1", "8"]}
// @@@MULTISNIP OnSendServerReadyComplete {"selectedLines": ["1", "9"]}
// @@@MULTISNIP OnAMSDrainReceived {"selectedLines": ["1", "11-12"]}
// @@@MULTISNIP OnAMSDrainReceivedTimer {"selectedLines": ["1", "21"]}
// @@@MULTISNIP OnV2SessionEnded {"selectedLines": ["1", "13"]}
// @@@MULTISNIP Helper {"selectedLines": ["1", "15-19"]}
private:
	void RegisterServer(const FName SessionName);
	void OnRegisterServerComplete(const bool bSucceeded);

	void UnregisterServer(const FName SessionName);
	void OnUnregisterServerComplete(const bool bSucceeded);

	void SendServerReady(const FName SessionName);
	void OnSendServerReadyComplete(const bool bSucceeded);

	void OnAMSDrainReceived();
	void ExecuteDrainLogic();
	void OnV2SessionEnded(const FName SessionName);

	TSharedPtr<FAccelByteModelsV2GameSession> GetSessionInfo(const FName SessionName) const;

	bool bServerAlreadyRegister;
	bool bUnregisterServerRunning;
	FOnlineSessionV2AccelBytePtr ABSessionInt;

	FTimerHandle DrainLogicTimerHandle;
// @@@SNIPEND
};
