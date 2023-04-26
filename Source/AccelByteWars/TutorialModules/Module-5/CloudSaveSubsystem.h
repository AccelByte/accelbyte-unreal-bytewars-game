// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModules/Module-5/CloudSaveLog.h"
#include "TutorialModules/Module-5/CloudSaveModels.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CloudSaveSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UCloudSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

#pragma region Module.5 Function Declarations
public:
	void SetPlayerRecord(const APlayerController* PC, const FString& RecordKey, const FJsonObject& RecordData, const FOnSetCloudSaveRecordComplete& OnSetRecordComplete);
	void GetPlayerRecord(const APlayerController* PC, const FString& RecordKey, const FOnGetCloudSaveRecordComplete& OnGetRecordComplete);

private:
	void OnSetPlayerRecordComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& Error, const FOnSetCloudSaveRecordComplete OnSetRecordComplete);
	void OnGetPlayerRecordComplete(int32 LocalUserNum, bool bWasSuccessful, const FAccelByteModelsUserRecord& UserRecord, const FString& Error, const FOnGetCloudSaveRecordComplete OnGetRecordComplete);
#pragma endregion

private:
	void BindDelegates();
	void UnbindDelegates();

	FDelegateHandle OnSetPlayerRecordCompletedDelegateHandle;
	FDelegateHandle OnGetPlayerRecordCompletedDelegateHandle;
	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
};