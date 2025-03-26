// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "DSHealthSubsystem.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogDSHealth, Log, All);

#define UE_LOG_DSHEALTH(Verbosity, Format, ...) \
{ \
	UE_LOG(LogDSHealth, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

#define DS_HEALTH_LOG_SHOW_PARAM TEXT("ShowDSHealthLog")
#define DS_HEALTH_LOG_INTERVAL_PARAM TEXT("DSHealthLogInterval")

UCLASS()
class ACCELBYTEWARS_API UDSHealthSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

private:
	void ShowDSHealthLog();
	void StartShowDSHealthLogTimer();
	void StopShowDSHealthLogTimer();

	void CheckCommandLineParam();

	bool bShowDSHealthLog = false;
	float DSHealthLogInterval = 1.0f;
	FTimerHandle ShowDSHealthLogHandle;
	FOnlineSessionV2AccelBytePtr ABSessionInt;
};
