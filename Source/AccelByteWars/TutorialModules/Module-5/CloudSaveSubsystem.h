// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModules/Module-5/CloudSaveLog.h"
#include "TutorialModules/Module-5/CloudSaveModels.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CloudSaveSubsystem.generated.h"

class UAccelByteWarsGameInstance;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UCloudSaveSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

#pragma region Module.5 Function Declarations
public:
	void SetPlayerRecord(const APlayerController* PC, const FString& RecordKey, const FJsonObject& RecordData, const FOnSetCloudSaveRecordComplete& OnSetRecordComplete);
	void GetPlayerRecord(const APlayerController* PC, const FString& RecordKey, const FOnGetCloudSaveRecordComplete& OnGetRecordComplete);
	void DeletePlayerRecord(const APlayerController* PC, const FString& RecordKey, const FOnDeleteCloudSaveRecordComplete& OnDeleteRecordComplete);

private:
	void OnSetPlayerRecordComplete(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FOnSetCloudSaveRecordComplete OnSetRecordComplete);
	void OnGetPlayerRecordComplete(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FAccelByteModelsUserRecord& UserRecord, const FOnGetCloudSaveRecordComplete OnGetRecordComplete);
	void OnDeletePlayerRecordComplete(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FOnDeleteCloudSaveRecordComplete OnDeleteRecordComplete);
#pragma endregion

private:
	void BindDelegates();
	void UnbindDelegates();

	void OnLoadGameSoundOptions(const APlayerController* PC, TDelegate<void()> OnComplete);
	void OnSaveGameSoundOptions(const APlayerController* PC, TDelegate<void()> OnComplete);

	FDelegateHandle OnSetPlayerRecordCompletedDelegateHandle;
	FDelegateHandle OnGetPlayerRecordCompletedDelegateHandle;
	FDelegateHandle OnDeletePlayerRecordCompletedDelegateHandle;
	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
};